cmake_minimum_required(VERSION 3.29)
project(OpenCV_lab5)

set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/opencv")  # Путь к папке opencv внутри проекта

find_package(OpenCV REQUIRED) #ищет библиотеку и хэдеры

set(CMAKE_CXX_STANDARD 17) # указывает стандарт языка

add_executable(OpenCV_lab5 main.cpp) #показывает какие файлы запускать так скажем


target_link_libraries(OpenCV_lab5 ${OpenCV_LIBS})

