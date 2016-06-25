VERSION_INFO = 1:0:0

MODULE_FLAGS =

if BUILD_SERIAL
MODULE_FLAGS+=-DMODULE_SERIAL
endif 

if BUILD_BLUETOOTH
MODULE_FLAGS+=-DMODULE_BLUETOOTH
endif

SUBDIRS=../libserialport

AM_CFLAGS = -I$(srcdir)/../libserialport/src
MY_COMMON_LIBS = ../libserialport/src/libserialport.la -lbluetooth

bin_PROGRAMS = main \
               phone_send phone_recv \
               serial_send serial_recv \
               blue_send blue_recv

lib_LTLIBRARIES = libphone.la \
                  libserial.la \
                  libblue.la \
                  libutils.la


# phone lib
libphone_la_SOURCES = phone.c
libphone_la_LIBADD = $(MY_COMMON_LIBS)
libphone_la_CFLAGS = $(MODULE_FLAGS)
# bluelib
libblue_la_SOURCES = bluetooth.c
libblue_la_LIBADD = $(MY_COMMON_LIBS)
libblue_la_CFLAGS = $(MODULE_FLAGS)
# serial lib
libserial_la_SOURCES = serial_server.c
libserial_la_LIBADD = $(MY_COMMON_LIBS)
libserial_la_CFLAGS = $(MODULE_FLAGS)
# utils lib
libutils_la_SOURCES = utils.c
libutils_la_LIBADD = $(MY_COMMON_LIBS)
libutils_la_CFLAGS = $(MODULE_FLAGS)

# main listen on linux
main_SOURCES = main.c
main_LDADD = libserial.la libblue.la libphone.la libutils.la -lbluetooth
main_CFLAGS = $(MODULE_FLAGS)


phone_send_SOURCES = phone.c
phone_send_LDADD = libutils.la libserial.la -lbluetooth
phone_send_CFLAGS = -DPHONE_SEND
phone_recv_SOURCES = phone.c
phone_recv_LDADD = libutils.la libserial.la -lbluetooth
phone_recv_CFLAGS = -DPHONE_RECV


blue_send_SOURCES = bluetooth.c
blue_send_LDADD = libutils.la libserial.la -lbluetooth
blue_send_CFLAGS = -DBLUETOOTH_SEND
blue_recv_SOURCES = bluetooth.c
blue_recv_LDADD = libutils.la libserial.la -lbluetooth
blue_recv_CFLAGS = -DBLUETOOTH_RECV


serial_recv_SOURCES = serial_server.c
serial_recv_LDADD = libutils.la -lbluetooth
serial_recv_CFLAGS = -DSERIAL_RECV
serial_send_SOURCES = serial_server.c
serial_send_LDADD = libutils.la -lbluetooth
serial_send_CFLAGS = -DSERIAL_SEND
