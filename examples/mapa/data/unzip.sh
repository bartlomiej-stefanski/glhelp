#!/bin/sh

for zip_file in *.zip; do
  unzip "${zip_file%.zip}"
  
  mv "${zip_file%.zip}"/* .
  
  rm -rf "${zip_file%.zip}"
done
