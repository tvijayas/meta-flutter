SUMMARY = "Flutter Engine"
DESCRIPTION = "Google Flutter Engine for use with Flutter applications"
AUTHOR = "Flutter Team"
HOMEPAGE = "https://github.com/flutter/engineflutter/"
BUGTRACKER = "https://github.com/flutter/flutter/issues"
SECTION = "graphics"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://flutter/LICENSE;md5=a60894397335535eb10b54e2fff9f265"
CVE_PRODUCT = "libflutter_engine.so"

REQUIRED_DISTRO_FEATURES = "opengl"

DEPENDS += "\
    ca-certificates-native \
    compiler-rt \
    curl-native \
    depot-tools-native \
    fontconfig \
    libcxx \
    zip-native \
    "

SRC_URI = "file://0001-clang-toolchain.patch \
           file://0002-x64-sysroot-assert.patch"

S = "${WORKDIR}/src"

inherit python3native features_check pkgconfig

require conf/include/gn-utils.inc

RUNTIME = "llvm"
TOOLCHAIN = "clang"
PREFERRED_PROVIDER:libgcc = "compiler-rt"

COMPATIBLE_MACHINE = "(-)"
COMPATIBLE_MACHINE:aarch64 = "(.*)"
COMPATIBLE_MACHINE:armv7 = "(.*)"
COMPATIBLE_MACHINE:armv7a = "(.*)"
COMPATIBLE_MACHINE:armv7ve = "(.*)"
COMPATIBLE_MACHINE:x86 = "(.*)"
COMPATIBLE_MACHINE:x86-64 = "(.*)"

FLUTTER_RUNTIME ??= "release"

PACKAGECONFIG ??= "disable-desktop-embeddings \
                   disable-embedder-examples \
                   embedder-for-target \
                   fontconfig \
                   ${FLUTTER_RUNTIME} \
                  "

PACKAGECONFIG[asan] = "--asan"
PACKAGECONFIG[coverage] = "--coverage"
PACKAGECONFIG[dart-debug] = "--dart-debug"
PACKAGECONFIG[debug] = "--runtime-mode debug"
PACKAGECONFIG[disable-desktop-embeddings] = "--disable-desktop-embeddings"
PACKAGECONFIG[disable-embedder-examples] = "--no-build-embedder-examples"
PACKAGECONFIG[disable-glfw-shell] = "--no-build-glfw-shell"
PACKAGECONFIG[disable-prebuilt-dart-sdk] = "--no-prebuilt-dart-sdk"
PACKAGECONFIG[embedder-examples] = "--build-embedder-examples"
PACKAGECONFIG[embedder-for-target] = "--embedder-for-target"
PACKAGECONFIG[fontconfig] = "--enable-fontconfig"
PACKAGECONFIG[full-dart-debug] = "--full-dart-debug"
PACKAGECONFIG[full-dart-sdk] = "--full-dart-sdk"
PACKAGECONFIG[glfw-shell] = "--build-glfw-shell"
PACKAGECONFIG[interpreter] = "--interpreter"
PACKAGECONFIG[jit_release] = "--runtime-mode jit_release"
PACKAGECONFIG[lsan] = "--lsan"
PACKAGECONFIG[msan] = "--msan"
PACKAGECONFIG[profile] = "--runtime-mode profile"
PACKAGECONFIG[release] = "--runtime-mode release"
PACKAGECONFIG[skshaper] = "--enable-skshaper"
PACKAGECONFIG[static-analyzer] = "--clang-static-analyzer"
PACKAGECONFIG[tsan] = "--tsan"
PACKAGECONFIG[ubsan] = "--ubsan"
PACKAGECONFIG[unoptimized] = "--unoptimized"
PACKAGECONFIG[vulkan] = "--enable-vulkan"
PACKAGECONFIG[vulkan-validation-layers] = "--enable-vulkan-validation-layers"


CLANG_TOOLCHAIN_TRIPLE = "${@gn_clang_triple_prefix(d)}"
CLANG_PATH = "${WORKDIR}/src/buildtools/linux-x64/clang"

ARGS_GN_FILE = "${WORKDIR}/src/${OUT_DIR_REL}/args.gn"

OUT_DIR_REL = "${@get_out_dir(d)}"

GN_ARGS = "${PACKAGECONFIG_CONFARGS} --clang --lto --no-goma --no-stripped "
GN_ARGS:append = " --target-os linux"
GN_ARGS:append = " --linux-cpu ${@gn_target_arch_name(d)}"
GN_ARGS:append = " --target-sysroot ${STAGING_DIR_TARGET}"
GN_ARGS:append = " --target-toolchain ${CLANG_PATH}"
GN_ARGS:append = " --target-triple ${CLANG_TOOLCHAIN_TRIPLE}"

GN_ARGS:append:armv7 = " --arm-float-abi ${TARGET_FPU}"
GN_ARGS:append:armv7a = " --arm-float-abi ${TARGET_FPU}"
GN_ARGS:append:armv7ve = " --arm-float-abi ${TARGET_FPU}"

ARGS_GN = ""
ARGS_GN:append:aarch64 = "arm_tune = \"${@gn_get_tune_features(d)}\""
ARGS_GN:append:armv7 = "arm_tune = \"${@gn_get_tune_features(d)}\""
ARGS_GN:append:armv7a = "arm_tune = \"${@gn_get_tune_features(d)}\""
ARGS_GN:append:armv7ve = "arm_tune = \"${@gn_get_tune_features(d)}\""

DEPOT_TOOLS ??= "${STAGING_DIR_NATIVE}/usr/share/depot_tools"
PYTHON2_PATH ??= "bootstrap-2@3.8.9.chromium.14_bin/python/bin"

FLUTTER_SDK_TAG ??= "${AUTOREV}"
FLUTTER_ENGINE_REPO_URL ??= "https://github.com/flutter/engine.git"
FLUTTER_ENGINE_COMMIT ??= "${@gn_get_engine_commit(d)}"
GCLIENT_ARGS_EXTRA = "${@gn_get_gclient_engine_revision(d)}"

do_patch:prepend() {

    export CURL_CA_BUNDLE=${STAGING_DIR_NATIVE}/etc/ssl/certs/ca-certificates.crt
    export PATH=${DEPOT_TOOLS}:${DEPOT_TOOLS}/${PYTHON2_PATH}:$PATH

    bbnote "FLUTTER_ENGINE_COMMIT: ${FLUTTER_ENGINE_COMMIT}"
    bbnote "ARGS: ${GN_ARGS}"
    bbnote "ARGS_GN_FILE: ${ARGS_GN_FILE}"
    bbnote "ARGS_GN: ${ARGS_GN}"
    bbnote "CLANG_TOOLCHAIN_TRIPLE: ${CLANG_TOOLCHAIN_TRIPLE}"
    bbnote "GCLIENT_ROOT: ${WORKDIR}"
    bbnote "gclient sync --shallow --no-history -R -D ${GCLIENT_ARGS_EXTRA} ${PARALLEL_MAKE}"
    bbnote "OUT_DIR_REL: ${OUT_DIR_REL}"
}

do_patch () {

    cd ${WORKDIR}

    rm -rf ${WORKDIR}/.cipd
    rm -rf ${WORKDIR}/.gclient*
    

    echo 'solutions = [
        {
            "managed" : False,
            "name" : "src/flutter",
            "url" : "${FLUTTER_ENGINE_REPO_URL}",
            "deps_file": "DEPS",
            "custom_vars" : {
                "download_android_deps" : False,
                "download_windows_deps" : False,
                "download_linux_deps"   : False,
            }
        }
    ]' > .gclient

    ##################################
    # depot_tools dependent variables
    ##################################

    # don't auto update...
    export DEPOT_TOOLS_UPDATE=0
    # force python 2
    export GCLIENT_PY3=0

    bbnote `python -V`
    bbnote `which python`
    bbnote `which gclient`

    #################################################################
    # --shallow:    Do a shallow clone into the cache dir
    # --no-history: No git history to minimize download
    # -R:           resets any local changes before updating
    # -D:           Deletes from the working copy any dependencies that
    #               have been removed since the last sync
    #################################################################
    gclient sync --shallow --no-history -R -D ${GCLIENT_ARGS_EXTRA} ${PARALLEL_MAKE}

    cd ${S}

    git apply ${WORKDIR}/0001-clang-toolchain.patch
    git apply ${WORKDIR}/0002-x64-sysroot-assert.patch

    # required object files to link
    CLANG_VERSION=`ls ${CLANG_PATH}/lib/clang`
    CLANG_LIB_TARGET_PATH=${CLANG_PATH}/lib/clang/${CLANG_VERSION}/lib/${CLANG_TOOLCHAIN_TRIPLE}
    mkdir -p ${CLANG_LIB_TARGET_PATH}
    cp ${STAGING_LIBDIR}/${TARGET_SYS}/*/crtbeginS.o ${CLANG_LIB_TARGET_PATH}/
    cp ${STAGING_LIBDIR}/${TARGET_SYS}/*/crtendS.o ${CLANG_LIB_TARGET_PATH}/
}
do_patch[depends] += "ca-certificates-native:do_populate_sysroot"
do_patch[depends] += "curl-native:do_populate_sysroot"
do_patch[depends] += "depot-tools-native:do_populate_sysroot"
do_patch[depends] += "fontconfig:do_populate_sysroot"

do_configure() {

    export PATH=${DEPOT_TOOLS}:${DEPOT_TOOLS}/${PYTHON2_PATH}:$PATH

    cd ${S}

    ./flutter/tools/gn ${GN_ARGS}

    echo ${ARGS_GN} >> ${ARGS_GN_FILE}
}
do_configure[depends] += "depot-tools-native:do_populate_sysroot"

do_compile() {

    export PATH=${DEPOT_TOOLS}:${DEPOT_TOOLS}/${PYTHON2_PATH}:$PATH

    cd ${S}

    autoninja -C ${OUT_DIR_REL}
}
do_compile[depends] += "depot-tools-native:do_populate_sysroot"
do_compile[progress] = "outof:^\[(\d+)/(\d+)\]\s+"

do_install() {

    install -d ${D}${libdir}
    install -m 644 ${S}/${OUT_DIR_REL}/so.unstripped/libflutter_engine.so ${D}${libdir}

    install -d ${D}${includedir}
    install -m 644 ${S}/${OUT_DIR_REL}/flutter_embedder.h ${D}${includedir}

    install -d ${D}${datadir}/flutter
    install -m 644 ${S}/${OUT_DIR_REL}/icudtl.dat ${D}${datadir}/flutter/

    # create SDK
    install -d ${D}${datadir}/flutter/sdk
    install -d ${D}${datadir}/flutter/sdk/clang_x64
    install -m 755 ${S}/${OUT_DIR_REL}/clang_x64/gen_snapshot ${D}${datadir}/flutter/sdk/clang_x64/

    # this avoids build break when SDK tag changes in middle of build
    cd ${S}/flutter
    echo `git rev-parse HEAD` > ${D}/usr/share/flutter/sdk/engine.version
    echo ${FLUTTER_ENGINE_REPO_URL} >> ${D}/usr/share/flutter/sdk/engine.version

    cd ${D}/${datadir}/flutter
    zip -r engine_sdk.zip sdk
    rm -rf sdk
}
do_install[depends] += "zip-native:do_populate_sysroot"

PACKAGES =+ "${PN}-sdk-dev"

FILES:${PN} = "\
    ${libdir} \
    ${datadir}/flutter/icudtl.dat \
    "

FILES:${PN}-sdk-dev = "\
    ${datadir}/flutter/engine_sdk.zip \
    "

FILES:${PN}-dev = "\
    ${includedir} \
    "

BBCLASSEXTEND = ""
