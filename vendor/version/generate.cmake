# Calculate the timestamp (runs every build)
string(TIMESTAMP PROJECT_BUILD_DATE "%d/%m/%Y - %H:%M:%S")

configure_file("${VERSION_TEMPLATE}" "${VERSION_SOURCE}" @ONLY)
