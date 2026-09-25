#!/bin/bash
#
# Build script for FloppyKernel (ginkgo).
# Based on build script for Quicksilver, by Ghostrider.
# Copyright (C) 2020-2021 Adithya R. (original version)
# Copyright (C) 2022-2025 Flopster101 (rewrite)
#
# Split into build/ckbuild.sh + build/scripts/* (mirrors flop_exynos2100_kernel/build).

## Logging helpers
source "$(pwd)/build/lib/log.sh"

## Variables
# Device fragments
GINKGO_FRAGMENT="vendor/ginkgo.config"
LAUREL_FRAGMENT="vendor/laurel_sprout.config"

# Parse device argument
if [[ -z "$1" ]]; then
    echo -e "\n$(log_err "Please specify device to build!")\n"
    exit 1
fi

TARGET_DEVICE="$1"
shift

# Set device-specific variables
case "$TARGET_DEVICE" in
    ginkgo)
        AK3_BRANCH="floppy-reborn"
        DEVICE="Redmi Note 8/8T"
        CODENAME="ginkgo"
        FRAGMENT="$GINKGO_FRAGMENT"
        ;;
    laurel_sprout)
        AK3_BRANCH="floppy-reborn-laurel"
        DEVICE="Xiaomi Mi A3"
        CODENAME="laurel_sprout"
        FRAGMENT="$LAUREL_FRAGMENT"
        ;;
    mitrinket)
        AK3_BRANCH="floppy-unity"
        DEVICE="Redmi Note 8/8T and Xiaomi Mi A3"
        CODENAME="mitrinket"
        FRAGMENT="vendor/unified.config"
        ;;
    *)
        echo -e "\n$(log_err "Unknown device: $TARGET_DEVICE")\n"
        exit 1
        ;;
esac

# Workspace
WP="${WP:-$(realpath "$PWD/../")}"

if [[ -z "$WP" ]]; then
    echo -e "\n$(log_err "Please set the WP env var.")\n"
    exit 1
fi

if [[ ! -d drivers ]]; then
    echo -e "\n$(log_err "Please execute from top-level kernel tree")\n"
    exit 1
fi

# Other
DEFAULT_DEFCONFIG="vendor/trinket-perf_defconfig"
BASE_FRAGMENT="vendor/xiaomi-trinket.config"
KERNEL_URL="https://github.com/Flopster101/flop_ginkgo_kernel"
AK3_URL="https://github.com/Flopster101/AnyKernel3"
SECONDS=0 # builtin bash timer
DATE="$(date '+%Y%m%d-%H%M')"
BUILD_HOST="$USER@$(hostname)"
# Paths
AK3_DIR="$WP/AnyKernel3"
KDIR="$(readlink -f .)"
USE_GCC_BINUTILS="0"
OUT_IMAGE="out/arch/arm64/boot/Image.gz-dtb"
DTBO_TMP="out/dtbotmp"
OUT_DTBO="$DTBO_TMP/dtbo.img"
# Set OUT_DTB based on device
if [[ "$CODENAME" == "laurel_sprout" ]]; then
    OUT_DTB="out/arch/arm64/boot/dts/xiaomi/laurel_sprout-trinket-base.dtb"
elif [[ "$CODENAME" == "mitrinket" ]]; then
    OUT_DTB_GINKGO="out/arch/arm64/boot/dts/xiaomi/qcom-base/trinket.dtb"
    OUT_DTB_LAUREL="out/arch/arm64/boot/dts/xiaomi/laurel_sprout-trinket-base.dtb"
    # Set OUT_DTB to ginkgo as default for checks
    OUT_DTB="$OUT_DTB_GINKGO"
else
    OUT_DTB="out/arch/arm64/boot/dts/xiaomi/qcom-base/trinket.dtb"
fi

IN_DTBO_GINKGO="out/arch/arm64/boot/dts/xiaomi/ginkgo-trinket-overlay.dtbo"
IN_DTBO_LAUREL="out/arch/arm64/boot/dts/xiaomi/laurel_sprout-trinket-overlay.dtbo"

## Customizable vars

# FloppyKernel version
FK_VER="v2.0b"

# Toggles
USE_CCACHE=1

# Droidspaces support
DROIDSPACES=1

## Parse arguments
DO_KSU=0
DO_SUKI=0
DO_XXKSU=0
DO_CLEAN=0
DO_MENUCONFIG=0
IS_RELEASE=0
DO_TG=0
DO_REGEN=0
DO_ZXZ=0
DO_FLTO=0
for arg in "$@"; do
    if [[ "$arg" == *m* ]]; then
        log_info "menuconfig argument passed, kernel configuration menu will be shown"
        DO_MENUCONFIG=1
    fi
    if [[ "$arg" == *k* ]]; then
        log_info "KernelSU argument passed, a KernelSU build will be made"
        DO_KSU=1
    fi
    if [[ "$arg" == *s* ]]; then
        log_info "ReSukiSU argument passed, a ReSukiSU build will be made"
        DO_SUKI=1
    fi
    if [[ "$arg" == *x* ]]; then
        log_info "XXKSU argument passed, an XXKSU build will be made"
        DO_XXKSU=1
    fi
    if [[ "$arg" == *c* ]]; then
        log_info "clean argument passed, output directory will be wiped"
        DO_CLEAN=1
    fi
    if [[ "$arg" == *R* ]]; then
        log_info "Release argument passed, build marked as release"
        IS_RELEASE=1
    fi
    if [[ "$arg" == *t* ]]; then
        log_info "Telegram argument passed, build will be uploaded to Telegram"
        DO_TG=1
    fi
    if [[ "$arg" == *o* ]]; then
        log_info "0x0.st upload enabled"
        DO_ZXZ=1
    fi
    if [[ "$arg" == *r* ]]; then
        log_info "config regeneration mode"
        DO_REGEN=1
    fi
    if [[ "$arg" == *l* ]]; then
        log_info "Full-LTO argument passed"
        log_warn "Full-LTO is VERY resource heavy and may take a long time to compile"
        DO_FLTO=1
    fi
done

KSU_COUNT=0
[ "$DO_KSU" == "1" ] && KSU_COUNT=$((KSU_COUNT + 1))
[ "$DO_SUKI" == "1" ] && KSU_COUNT=$((KSU_COUNT + 1))
[ "$DO_XXKSU" == "1" ] && KSU_COUNT=$((KSU_COUNT + 1))
if [ "$KSU_COUNT" -gt 1 ]; then
    log_err "Multiple SU variants are mutually exclusive. Please select only one."
    exit 1
fi

DEFCONFIG="$DEFAULT_DEFCONFIG"
if [[ "$IS_RELEASE" == "1" ]]; then
    BUILD_TYPE="Release"
else
    BUILD_TYPE="Testing"
fi

TEST_CHANNEL=1
#TEST_BUILD=0

# Upload build log
LOG_UPLOAD=1

## Secrets
if [[ "$TEST_CHANNEL" == "0" ]]; then
    TELEGRAM_CHAT_ID="$(cat ../chat)"
elif [[ "$TEST_CHANNEL" == "1" ]]; then
    TELEGRAM_CHAT_ID="$(cat ../chat_test)"
fi
TELEGRAM_BOT_TOKEN="$(cat ../bot_token)"

## Build type
LINUX_VER=$(make kernelversion 2>/dev/null)

if [[ "$IS_RELEASE" == "1" ]]; then
    BUILD_TYPE="Release"
else
    BUILD_TYPE="Testing"
fi

CK_TYPE=""
CK_TYPE_SHORT=""
if [[ "$DO_KSU" == "1" ]]; then
    CK_TYPE="KSUNext"
    CK_TYPE_SHORT="KN"
elif [ "$DO_SUKI" == "1" ]; then
    CK_TYPE="ReSukiSU-SUSFS"
    CK_TYPE_SHORT="RESKS"
elif [ "$DO_XXKSU" == "1" ]; then
    CK_TYPE="XXKSU-SUSFS"
    CK_TYPE_SHORT="XXK"
else
    CK_TYPE="Vanilla"
    CK_TYPE_SHORT="V"
fi
ZIP_PATH="$KDIR/build/Floppy_$FK_VER-$CK_TYPE-$CODENAME-$DATE.zip"

echo -e "\n$(log_info "Build info:")
- Device: $DEVICE ($CODENAME)
- Addons: $CK_TYPE
- FloppyKernel version: $FK_VER
- Linux version: $LINUX_VER
- Defconfig: $DEFCONFIG
- Build date: $DATE
- Build type: $BUILD_TYPE
- Clean build: $([ "$DO_CLEAN" -eq 1 ] && echo "Yes" || echo "No")
"

## Source split scripts
SCRIPTS_DIR="build/scripts"

# Pre-build dependencies (installs distro deps on source)
source "$SCRIPTS_DIR/deps.sh"

# Toolchain (sets up dirs + downloads + preps on source)
source "$SCRIPTS_DIR/tc.sh"

# Build, post-build, images and upload functions
source "$SCRIPTS_DIR/build.sh"
source "$SCRIPTS_DIR/post.sh"
source "$SCRIPTS_DIR/images.sh"
source "$SCRIPTS_DIR/upload.sh"

prep_build() {
    ## Prepare ccache
    if [[ "$USE_CCACHE" == "1" ]]; then
        log_info "Using ccache"
    fi

    # Show compiler info
    echo -e "$(log_info "Compiler: $KBUILD_COMPILER_STRING")\n"
}

clean() {
    make O=out clean
    make O=out mrproper
}

## Run build
# Do a clean build?
if [[ "$DO_CLEAN" == "1" ]]; then
    clean
fi
prep_build
build

dtbo_build
post_build
clean_tmp

upload