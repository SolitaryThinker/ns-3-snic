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
                 model/packet-buffer.cc
                 model/snic-scheduler-header.cc
                 model/snic-scheduler.cc
                 utils/flow.cc
                 utils/packet-arrival-rate-gen.cc
                 utils/ring-topology.cc
                 utils/statistic.cc
                 #model/snic-channel.cc
                 helper/snic-helper.cc
    HEADER_FILES model/snic-net-device.h
                 model/network-task-addn.h
                 model/network-task.h
                 model/packet-buffer.h
                 model/snic-scheduler-header.h
                 model/snic-scheduler.h
                 utils/flow.h
                 utils/packet-arrival-rate-gen.h
                 utils/ring-topology.h
                 utils/statistic.h
                 #model/snic-header.h
                 #model/snic-channel.h
                 helper/snic-helper.h
    LIBRARIES_TO_LINK ${libcore}
                      ${libinternet}
                      ${libcsma}
                      #${libnetwork}
    TEST_SOURCES test/snic-test-suite.cc
                 ${examples_as_tests_sources}
)

