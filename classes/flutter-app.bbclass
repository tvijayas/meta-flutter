# Helper class for building Flutter Application.
# Assumes that:
# - Flutter Application does not have a linux folder.  If it does it 
#   will incorrectly link to Linux GTK embedder.  We don't want that.
# - S is defined and points to source directory.
# - PUBSPEC_APPNAME is defined correctly.  This is the name value from pubspec.yml.
#   TODO -> read pubspec.yml via python

DEPENDS += " \
    ca-certificates-native \
    flutter-sdk-native \
    unzip-native \
    "

FLUTTER_RUNTIME ??= "release"

FLUTTER_APPLICATION_PATH ??= "."

FLUTTER_EXTRA_BUILD_ARGS ??= ""

#
# Build flutter_assets folder and AOT (libapp.so)
#

do_compile() {

    FLUTTER_SDK=${STAGING_DIR_NATIVE}/usr/share/flutter/sdk
    ENGINE_SDK=${S}/engine_sdk/sdk

    export PATH=${FLUTTER_SDK}/bin:$PATH

    cd ${S}/${FLUTTER_APPLICATION_PATH}

    flutter build bundle ${FLUTTER_EXTRA_BUILD_ARGS}

    if ${@bb.utils.contains('FLUTTER_RUNTIME', 'release', 'true', 'false', d)} || \
       ${@bb.utils.contains('FLUTTER_RUNTIME', 'profile', 'true', 'false', d)}; then

        PROFILE_ENABLE=false
        if ${@bb.utils.contains('FLUTTER_RUNTIME', 'profile', 'true', 'false', d)}; then
            PROFILE_ENABLE=true
        fi

        ${FLUTTER_SDK}/bin/cache/dart-sdk/bin/dart \
            --verbose \
            --disable-analytics \
            --disable-dart-dev ${FLUTTER_SDK}/bin/cache/artifacts/engine/linux-x64/frontend_server.dart.snapshot \
            --sdk-root ${FLUTTER_SDK}/bin/cache/artifacts/engine/common/flutter_patched_sdk_product/ \
            --target=flutter \
            --no-print-incremental-dependencies \
            -Ddart.vm.profile=${PROFILE_ENABLE} \
            -Ddart.vm.product=true \
            --aot --tfa \
            --packages .dart_tool/package_config.json \
            --output-dill .dart_tool/flutter_build/*/app.dill \
            --depfile .dart_tool/flutter_build/*/kernel_snapshot.d \
            package:${PUBSPEC_APPNAME}/main.dart            

        #
        # Extract Engine SDK
        #
        rm -rf ${S}/engine_sdk
        unzip ${STAGING_DATADIR}/flutter/engine_sdk.zip -d ${S}/engine_sdk

        #
        # Create libapp.so
        #
        ${S}/engine_sdk/sdk/clang_x64/gen_snapshot \
            --snapshot_kind=app-aot-elf \
            --elf=libapp.so \
            --strip \
            .dart_tool/flutter_build/*/app.dill
    fi
}

INSANE_SKIP:${PN} += " ldflags libdir"
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_install() {
    install -d ${D}${datadir}/${PUBSPEC_APPNAME}
    if ${@bb.utils.contains('FLUTTER_RUNTIME', 'release', 'true', 'false', d)} || \
       ${@bb.utils.contains('FLUTTER_RUNTIME', 'profile', 'true', 'false', d)}; then
        cp ${S}/${FLUTTER_APPLICATION_PATH}/libapp.so ${D}${datadir}/${PUBSPEC_APPNAME}/
    fi
    cp -r ${S}/${FLUTTER_APPLICATION_PATH}/build/flutter_assets/* ${D}${datadir}/${PUBSPEC_APPNAME}/
}

FILES:${PN} = "${datadir}"
FILES:${PN}-dev = ""