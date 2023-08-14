require "mkmf"

unless find_header("sqlite3.h") && find_library("sqlite3", "sqlite3_libversion_number", "sqlite3.h")
  abort "sqlite3 is missing"
end
unless find_library("curl", "curl_multi_init")
  abort "curl is missing"
end

create_makefile("cloudsqlite")
