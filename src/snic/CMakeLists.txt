check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/snic-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME snic
    SOURCE_FILES model/snic-net-device.cc
                 #model/snic-header.cc
                 model/network-task-addn.cc
                 model/network-task.cc
                 #model/snic-channel.cc
                 helper/snic-helper.cc
    HEADER_FILES model/snic-net-device.h
                 model/network-task-addn.h
                 model/network-task.h
                 #model/snic-header.h
                 #model/snic-channel.h
                 helper/snic-helper.h
    LIBRARIES_TO_LINK ${libcore}
                      ${libinternet}
                      #${libnetwork}
    TEST_SOURCES test/snic-test-suite.cc
                 ${examples_as_tests_sources}
)

