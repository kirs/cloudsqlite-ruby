require "mkmf"

unless find_header("yaml.h") && find_library("yaml", "yaml_get_version")
  abort("\nERROR: *** could not find libyaml development environment ***\n\n")
end
unless find_header("sqlite3.h") && find_library("sqlite3", "sqlite3_libversion_number", "sqlite3.h")
  abort "damn"
end
unless find_library("curl", "curl_multi_init")
  abort "ddd"
end

create_makefile("cloudsqlite")
