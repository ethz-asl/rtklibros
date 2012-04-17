cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

set(ROS_BUILD_TYPE Debug)

rosbuild_init()

add_definitions("-Wall -ansi -pedantic -I$(SRC) -DTRACE -lpthread
 -DENAGLO -DENAQZS -DENAGAL -DNFREQ=3")

link_libraries(m)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

rosbuild_genmsg()

include_directories ("${PROJECT_SOURCE_DIR}/src")

#pos2kml
rosbuild_add_library(convkml src/convkml.c)
rosbuild_add_library(solution src/solution.c)
rosbuild_add_library(geoid src/geoid.c)
rosbuild_add_library(rtkcmn src/rtkcmn.c)
rosbuild_add_library(solution src/solution.c)

rosbuild_add_executable(pos2kml app/pos2kml/pos2kml.c)
target_link_libraries(pos2kml convkml solution geoid rtkcmn solution)

#str2str
rosbuild_add_library(ss2 src/rcv/ss2.c)
rosbuild_add_library(crescent src/rcv/crescent.c)
rosbuild_add_library(skytraq src/rcv/skytraq.c)
rosbuild_add_library(gw10 src/rcv/gw10.c)
rosbuild_add_library(javad src/rcv/javad.c)
rosbuild_add_library(stream src/stream.c)
rosbuild_add_library(rtkcmn src/rtkcmn.c)
rosbuild_add_library(sbas src/sbas.c)
rosbuild_add_library(geoid src/geoid.c)
rosbuild_add_library(ublox src/rcv/ublox.c src/rcvraw.c)
target_link_libraries(ublox ss2 crescent gw10 javad skytraq crescent sbas novatel rtkcmn rinex rtcm)
rosbuild_add_library(novatel src/rcv/novatel.c)
rosbuild_add_library(streamsvr src/streamsvr.c)

rosbuild_add_executable(str2str app/str2str/str2str.c)
target_link_libraries(str2str stream rtkcmn solution geoid ublox skytraq streamsvr)

#rnx2rtkp

rosbuild_add_library(rtkcmn  src/rtkcmn.c)
rosbuild_add_library(rinex  src/rinex.c)
rosbuild_add_library(rtkpos  src/rtkpos.c)
target_link_libraries(rtkpos lambda ppp pntpos)
rosbuild_add_library(postpos  src/postpos.c)
target_link_libraries(postpos sbas rtkpos rtcm rinex)
rosbuild_add_library(solution src/solution.c)

rosbuild_add_library(lambda  src/lambda.c)
rosbuild_add_library(preceph  src/preceph.c)
rosbuild_add_library(pntpos  src/pntpos.c)
rosbuild_add_library(ephemeris  src/ephemeris.c)
rosbuild_add_library(options src/options.c)
rosbuild_add_library(ppp  src/ppp.c)
target_link_libraries(ppp pntpos)
rosbuild_add_library(rtcm  src/rtcm.c)
rosbuild_add_library(ionex src/ionex.c)

rosbuild_add_executable(rnx2rtkp app/rnx2rtkp/rnx2rtkp.c)
target_link_libraries(rnx2rtkp rtkcmn postpos solution ionex
geoid preceph pntpos ephemeris options rtcm ionex)

#convbin
rosbuild_add_library(convrnx src/convrnx.c)
target_link_libraries(convrnx rtkcmn sbas rtkpos rtcm rinex)

rosbuild_add_executable(convbin app/convbin/convbin.c src/rcvraw.c)
target_link_libraries(convbin ublox rtkcmn rinex sbas convrnx rtcm skytraq 
rtkcmn rtksvr rtkpos geoid solution lambda
sbas stream rtcm preceph options pntpos ppp ublox 
 ephemeris rinex ionex)
	
#rtkrcv

rosbuild_add_library(rtksvr src/rtksvr.cc)
rosbuild_add_executable(rtkrcv app/rtkrcv/rtkrcv.cc)
target_link_libraries(rtkrcv rtkcmn rtksvr rtkpos geoid solution lambda
sbas stream rtcm preceph options pntpos ppp ublox 
 ephemeris rinex ionex)





