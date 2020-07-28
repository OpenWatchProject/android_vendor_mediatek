SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(LIBDIR "${PREFIX}/lib")
SET(RESOURCEDIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

set(CMAKE_VERBOSE_MAKEFILE ON)

pkg_check_modules(rpkgs REQUIRED libxml2)
pkg_check_modules(rpkgs REQUIRED libxml2-devel)
pkg_check_modules(rpkgs REQUIRED libwbxml2)
pkg_check_modules(rpkgs REQUIRED libcurl-devel)
pkg_check_modules(rpkgs REQUIRED openssl-devel)
add_definitions(${rpkgs_CFLAGS})

SET(SRCS
        Common/base64.c
        Common/commandhandler.c
        Common/contentmanager.c
        Common/pacmp3.c
        Common/pacmtl.c
        Common/provisioningengine.c
        Common/registry.c
        Common/seclient.c
        Common/trustletchannel.c
        Common/xmlmessagehandler.c
)

INCLUDE_DIRECTORIES(Common/include)
INCLUDE_DIRECTORIES(Common)
INCLUDE_DIRECTORIES(/usr/include/libxml2)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -Wconversion-null -Werror=strict-aliasing")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DLINUX -DARM -D_32BIT -DANDROID_ARM=1 -DTIZEN")

# Rules for LIBPROVISIONINGAGENT
SET(LIBPROVISIONINGAGENT_SO "provisioningagent")
ADD_LIBRARY(${LIBPROVISIONINGAGENT_SO} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${LIBPROVISIONINGAGENT_SO} ${rpkgs_LDFLAGS} ${LIBCLIENT_SO} ${LIBREGISTRY_SO} -lssl -lcrypto -lz -lxml2 -lcurl)

INSTALL(TARGETS ${LIBPROVISIONINGAGENT_SO} DESTINATION ${LIBDIR})
INSTALL(DIRECTORY Common/include/ DESTINATION /usr/include FILES_MATCHING PATTERN "*.h")
