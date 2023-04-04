build_lib_example(
    NAME snic-example
    SOURCE_FILES snic-example.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-app
    SOURCE_FILES snic-example-app.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-top
    SOURCE_FILES snic-example-top.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-top-4
    SOURCE_FILES snic-example-top-4.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-top-route
    SOURCE_FILES snic-example-top-route.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-top-helper
    SOURCE_FILES snic-example-top-helper.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-example-flow
    SOURCE_FILES snic-example-flow.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)
build_lib_example(
    NAME snic-example-workload
    SOURCE_FILES snic-example-workload.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)


build_lib_example(
    NAME snic-example-socket
    SOURCE_FILES snic-example-socket.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-sched
    SOURCE_FILES snic-sched.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)

build_lib_example(
    NAME snic-eval
    SOURCE_FILES snic-eval.cc
    LIBRARIES_TO_LINK
        ${libsnic}
        ${libapplications}
        ${libcsma}
        ${libinternet}
)
