VERSION_INFO = 1:0:0

MY_COMMON_LIBS = -lserialport -lbluetooth
MODULE_FLAGS =

if BUILD_SERIAL
MODULE_FLAGS+=-DMODULE_SERIAL
endif 

if BUILD_BLUETOOTH
MODULE_FLAGS+=-DMODULE_BLUETOOTH
endif

bin_PROGRAMS = main \
               phone_recv phone_send \
               serial_recv serial_send \
               blue_recv blue_send

main_SOURCES= main.c utils.c serial_server.c bluetooth.c
main_LDADD = $(MY_COMMON_LIBS)
main_CFLAGS = $(MODULE_FLAGS)

phone_recv_SOURCES = phone.c utils.c serial_server.c
phone_recv_LDADD = $(MY_COMMON_LIBS)
phone_recv_CFLAGS = -DPHONE_RECV $(MODULE_FLAGS)

phone_send_SOURCES = phone.c utils.c serial_server.c
phone_send_LDADD = $(MY_COMMON_LIBS)
phone_send_CFLAGS = -DPHONE_SEND $(MODULE_FLAGS)

blue_recv_SOURCES = bluetooth.c utils.c serial_server.c
blue_recv_LDADD = $(MY_COMMON_LIBS)
blue_recv_CFLAGS = -DBLUETOOTH_RECV $(MODULE_FLAGS)

blue_send_SOURCES = bluetooth.c utils.c serial_server.c
blue_send_LDADD = $(MY_COMMON_LIBS)
blue_send_CFLAGS = -DBLUETOOTH_SEND $(MODULE_FLAGS)

serial_recv_SOURCES = utils.c serial_server.c
serial_recv_LDADD = $(MY_COMMON_LIBS)
serial_recv_CFLAGS = -DSERIAL_RECV $(MODULE_FLAGS)

serial_send_SOURCES = utils.c serial_server.c
serial_send_LDADD = $(MY_COMMON_LIBS)
serial_send_CFLAGS = -DSERIAL_SEND $(MODULE_FLAGS)
