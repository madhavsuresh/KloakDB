add_library(dataformat postgres_client.cpp)

add_library(pqxxcompat pqxx_compat.cpp)
target_link_libraries(pqxxcompat pqxx pq dataformat)
