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
