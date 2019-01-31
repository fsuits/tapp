# Copyright 2019, IBM Corporation
# 
# This source code is licensed under the Apache License, Version 2.0 found in
# the LICENSE.md file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.1)

include_directories("${PROJECT_SOURCE_DIR}" ./)

add_library(TAPPLib Mesh/DoubleMatrix.cpp Mesh/StringTokenizer.cpp
Utilities/Attribute.cpp Utilities/EventHandler.cpp Utilities/Sanitization.cpp Utilities/StringManipulation.cpp
Collections/AttributeMap.cpp
Exceptions/FileAccessError.cpp Exceptions/FormatError.cpp
Filetypes/FileRelations/FileRelations.cpp Filetypes/Mzid/Mzid_File.cpp
Filetypes/Mzid/Mzid_Parser.cpp Filetypes/MzXML/MzXML_File.cpp Filetypes/MzXML/MzXML_Parser.cpp
Filetypes/TAPP/Header.cpp Filetypes/TAPP/IPL.cpp Filetypes/TAPP/MPKS.cpp Filetypes/TAPP/PID.cpp
Filetypes/TAPP/PKS.cpp Filetypes/TAPP/TAPP_Output.cpp
IO/BufferedWriter.cpp IO/FileReader.cpp IO/FileWriter.cpp IO/ManagedParameterization.cpp IO/StreamReading.cpp
MassSpectrometry/RelationalTables/DataExtraction.cpp MassSpectrometry/RelationalTables/LinkedTables.cpp
MassSpectrometry/RelationalTables/TableInitialization.cpp
)


