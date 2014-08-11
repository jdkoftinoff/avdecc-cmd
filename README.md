avdecc-cmd
==========

Command line tools for simple IEEE Std 1722.1-2013 messages
Copyright 2014 By J.D. Koftinoff Software, Ltd.
www.jdkoftinoff.com
jeffk@jdkoftinoff.com


Overview
--------

This source package when compiled builds the following command line tools:

* **avdecc** - general message generation
* **avdecc-discover** - trigger discover of entities
* **avdecc-read-descriptor** - read a specific descriptor from an entity
* **avdecc-get-control** - get a control's value from an entity
* **avdecc-set-control** - set a control value on an entity


Getting the Source Code
------------------------

```sh
git clone https://github.com/jdkoftinoff/avdecc-cmd.git --recursive
cd avdecc-cmd
```

Compiling
----------

### For Linux

#### With cmake

See [http://www.cmake.org/](http://www.cmake.org/) for info on cmake.

To build and install:

```sh
cmake -G "Unix Makefiles"
make
```

#### With configure

To build and install to /usr/local/bin

```sh
./configure --target-platform-linux=1 --prefix=/usr/local
make
sudo make install
```


### For Mac OS X

#### With cmake for Xcode

See [http://www.cmake.org/](http://www.cmake.org/) for info on cmake.

```sh
cmake -G "Xcode"
open avdecc-cmd.xcodeproj
```

#### With cmake for Makefiles

```sh
cmake -G "Unix Makefiles"
make
```

#### With configure

To build and install to /usr/local/bin

```sh
./configure --target-platform-macosx=1 --prefix=/usr/local
make
sudo make install
```

Using avdecc-cmd as a library in your program
---------------------------------------------

### Linux

* Add the directories ``jdksavdecc-c/include`` and ``include`` to your compiler's include path
* Add all C files from ``jdksavdecc-c/src`` and ``src`` to your project

### Mac OS X

* Add the directories ``jdksavdecc-c/include`` and ``include`` to your compiler's include path
* Add all C files from ``jdksavdecc-c/src`` and ``src`` to your project
* Link libpcap to your project by passing ``-lpcap`` on the linker command line

Usage
-----

All programs need either root/administrator access or the appropriate capability 
set to access raw ethernet frames via the desired ethernet driver.

All programs have the following common first 3 parameters:

1. **verbosity** - A number either 0, 1, or 2
2. **timeout_in_ms** - The number of milliseconds to wait for a response
3. **network_port** - The ethernet port to use

For example, running **avdecc-discover** on linux:

```sh
avdecc-discover 2 1000 eth0 
```

or on Mac OS X:

```sh
avdecc-discover 2 1000 en0
``` 


### avdecc

```
avdecc usage:
	avdecc [verbosity] [timeout_in_ms] [network_port] [protocol] ...

	avdecc [verbosity] [timeout_in_ms] [network_port] adp [message_type] (entity_id)

	avdecc [verbosity] [timeout_in_ms] [network_port] acmp [message_type] [sequence_id] [talker_entity_id]
		[talker_unique_id] [listener_entity_id] [listener_unique_id]

	avdecc [verbosity] [timeout_in_ms] [network_port] acmp [message_type] [sequence_id] [talker_entity_id]
		[talker_unique_id] [listener_entity_id] [listener_unique_id] [connection_count]

	avdecc [verbosity] [timeout_in_ms] [network_port] aecp AEM_COMMAND [command] [destination_mac] [target_entity_id] [sequence_id] 
		[payload...]
```

Options for ADP message types may be one of the following symbols:

```
	0x0000 ENTITY_AVAILABLE
	0x0001 ENTITY_DEPARTING
	0x0002 ENTITY_DISCOVER
```

Options for ACMP message types may be one of the following symbols:

```
	0x0000 CONNECT_TX_COMMAND
	0x0001 CONNECT_TX_RESPONSE
	0x0002 DISCONNECT_TX_COMMAND
	0x0003 DISCONNECT_TX_RESPONSE
	0x0004 GET_TX_STATE_COMMAND
	0x0005 GET_TX_STATE_RESPONSE
	0x0006 CONNECT_RX_COMMAND
	0x0007 CONNECT_RX_RESPONSE
	0x0008 DISCONNECT_RX_COMMAND
	0x0009 DISCONNECT_RX_RESPONSE
	0x000a GET_RX_STATE_COMMAND
	0x000b GET_RX_STATE_RESPONSE
	0x000c GET_TX_CONNECTION_COMMAND
	0x000d GET_TX_CONNECTION_RESPONSE
```

Options for AECP message types may be one of the following symbols:

```
	0x0000 AEM_COMMAND
	0x0001 AEM_RESPONSE
	0x0002 ADDRESS_ACCESS_COMMAND
	0x0003 ADDRESS_ACCESS_RESPONSE
	0x0004 AVC_COMMAND
	0x0005 AVC_RESPONSE
	0x0006 VENDOR_UNIQUE_COMMAND
	0x0007 VENDOR_UNIQUE_RESPONSE
	0x0008 HDCP_APM_COMMAND
	0x0009 HDCP_APM_RESPONSE
	0x000e EXTENDED_COMMAND
	0x000f EXTENDED_RESPONSE
```

Options for AECP AEM\_COMMAND and AEM\_RESPONSE command may be one of the following symbols:

```
	0x0000 ACQUIRE_ENTITY
	0x0001 LOCK_ENTITY
	0x0002 ENTITY_AVAILABLE
	0x0003 CONTROLLER_AVAILABLE
	0x0004 READ_DESCRIPTOR
	0x0005 WRITE_DESCRIPTOR
	0x0006 SET_CONFIGURATION
	0x0007 GET_CONFIGURATION
	0x0008 SET_STREAM_FORMAT
	0x0009 GET_STREAM_FORMAT
	0x000a SET_VIDEO_FORMAT
	0x000b GET_VIDEO_FORMAT
	0x000c SET_SENSOR_FORMAT
	0x000d GET_SENSOR_FORMAT
	0x000e SET_STREAM_INFO
	0x000f GET_STREAM_INFO
	0x0010 SET_NAME
	0x0011 GET_NAME
	0x0012 SET_ASSOCIATION_ID
	0x0013 GET_ASSOCIATION_ID
	0x0014 SET_SAMPLING_RATE
	0x0015 GET_SAMPLING_RATE
	0x0016 SET_CLOCK_SOURCE
	0x0017 GET_CLOCK_SOURCE
	0x0018 SET_CONTROL
	0x0019 GET_CONTROL
	0x001a INCREMENT_CONTROL
	0x001b DECREMENT_CONTROL
	0x001c SET_SIGNAL_SELECTOR
	0x001d GET_SIGNAL_SELECTOR
	0x001e SET_MIXER
	0x001f GET_MIXER
	0x0020 SET_MATRIX
	0x0021 GET_MATRIX
	0x0022 START_STREAMING
	0x0023 STOP_STREAMING
	0x0024 REGISTER_UNSOLICITED_NOTIFICATION
	0x0025 DEREGISTER_UNSOLICITED_NOTIFICATION
	0x0026 IDENTIFY_NOTIFICATION
	0x0027 GET_AVB_INFO
	0x0028 GET_AS_PATH
	0x0029 GET_COUNTERS
	0x002a REBOOT
	0x002b GET_AUDIO_MAP
	0x002c ADD_AUDIO_MAPPINGS
	0x002d REMOVE_AUDIO_MAPPINGS
	0x002e GET_VIDEO_MAP
	0x002f ADD_VIDEO_MAPPINGS
	0x0030 REMOVE_VIDEO_MAPPINGS
	0x0031 GET_SENSOR_MAP
	0x0032 ADD_SENSOR_MAPPINGS
	0x0033 REMOVE_SENSOR_MAPPINGS
	0x0034 START_OPERATION
	0x0035 ABORT_OPERATION
	0x0036 OPERATION_STATUS
	0x0037 AUTH_ADD_KEY
	0x0038 AUTH_DELETE_KEY
	0x0039 AUTH_GET_KEY_LIST
	0x003a AUTH_GET_KEY
	0x003b AUTH_ADD_KEY_TO_CHAIN
	0x003c AUTH_DELETE_KEY_FROM_CHAIN
	0x003d AUTH_GET_KEYCHAIN_LIST
	0x003e AUTH_GET_IDENTITY
	0x003f AUTH_ADD_TOKEN
	0x0040 AUTH_DELETE_TOKEN
	0x0041 AUTHENTICATE
	0x0042 DEAUTHENTICATE
	0x0043 ENABLE_TRANSPORT_SECURITY
	0x0044 DISABLE_TRANSPORT_SECURITY
	0x0045 ENABLE_STREAM_ENCRYPTION
	0x0046 DISABLE_STREAM_ENCRYPTION
	0x0047 SET_MEMORY_OBJECT_LENGTH
	0x0048 GET_MEMORY_OBJECT_LENGTH
	0x0049 SET_STREAM_BACKUP
	0x004a GET_STREAM_BACKUP
	0x7fff EXPANSION
```


#### examples


### avdecc-discover


```
  avdecc-discover usage:
	avdecc-discover [verbosity] [timeout_in_ms] [network_port]
```

Example run:


```
$ avdecc-discover 0 250 en0
Received ADPDU:
                                      DA:91-E0-F0-01-00-00
                                      SA:00-1C-AB-00-05-3A
                               EtherType:0x22F0
                          Payload Length:0x0044
                            message_type:ENTITY_AVAILABLE
                    valid_time (seconds):0x0028
                     control_data_length:0x0038
                               entity_id:00:1C:AB:FF:FE:00:05:3A
                         available_index:0x000083F3
                         entity_model_id:00:1C:AB:90:03:00:30:00
                     entity_capabilities:[ AEM_SUPPORTED CLASS_A_SUPPORTED GPTP_SUPPORTED AEM_IDENTIFY_CONTROL_INDEX_VALID AEM_INTERFACE_INDEX_VALID ]
                   talker_stream_sources:0x0006
                     talker_capabilities:[ IMPLEMENTED MEDIA_CLOCK_SOURCE AUDIO_SOURCE ]
                   listener_stream_sinks:0x0006
                   listener_capabilities:[ IMPLEMENTED MEDIA_CLOCK_SINK AUDIO_SINK ]
                     gptp_grandmaster_id:00:04:96:FF:FE:6D:79:28
                      gptp_domain_number:0x0000
                  identify_control_index:0x0000
                         interface_index:0x0001

```
	
### avdecc-read-descriptor


```
avdecc-read-descriptor usage:
	avdecc-read-descriptor [verbosity] [timeout_in_ms] [network_port] [sequence_id] [destination_mac] [target_entity_id] [descriptor_type] [descriptor_index]
```

The descriptor_type parameter has the following options:

```
	0x0000 ENTITY
	0x0001 CONFIGURATION
	0x0002 AUDIO_UNIT
	0x0003 VIDEO_UNIT
	0x0004 SENSOR_UNIT
	0x0005 STREAM_INPUT
	0x0006 STREAM_OUTPUT
	0x0007 JACK_INPUT
	0x0008 JACK_OUTPUT
	0x0009 AVB_INTERFACE
	0x000a CLOCK_SOURCE
	0x000b MEMORY_OBJECT
	0x000c LOCALE
	0x000d STRINGS
	0x000e STREAM_PORT_INPUT
	0x000f STREAM_PORT_OUTPUT
	0x0010 EXTERNAL_PORT_INPUT
	0x0011 EXTERNAL_PORT_OUTPUT
	0x0012 INTERNAL_PORT_INPUT
	0x0013 INTERNAL_PORT_OUTPUT
	0x0014 AUDIO_CLUSTER
	0x0015 VIDEO_CLUSTER
	0x0016 SENSOR_CLUSTER
	0x0017 AUDIO_MAP
	0x0018 VIDEO_MAP
	0x0019 SENSOR_MAP
	0x001a CONTROL
	0x001b SIGNAL_SELECTOR
	0x001c MIXER
	0x001d MATRIX
	0x001e MATRIX_SIGNAL
	0x001f SIGNAL_SPLITTER
	0x0020 SIGNAL_COMBINER
	0x0021 SIGNAL_DEMULTIPLEXER
	0x0022 SIGNAL_MULTIPLEXER
	0x0023 SIGNAL_TRANSCODER
	0x0024 CLOCK_DOMAIN
	0x0025 CONTROL_BLOCK
	0xffff INVALID
```

Example run:

```
$ sudo ./tmp-target/build/tools/avdecc-read-descriptor 0 500 eth1 0x0000 00-1C-AB-00-05-3A 00:1C:AB:FF:FE:00:05:3A ENTITY 0x0000
Response: 
                                      DA:BC-AE-C5-75-3C-19
                                      SA:00-1C-AB-00-05-3A
                               EtherType:0x22F0
                          Payload Length:0x0154
                            message_type:AEM_RESPONSE
                                  status:SUCCESS
                     control_data_length:0x0148
                        target_entity_id:00:1C:AB:FF:FE:00:05:3A
                    controller_entity_id:BC:AE:C5:FF:FE:75:3C:19
                             sequence_id:0x0000
                         u (unsolicited):false
                            command_type:READ_DESCRIPTOR
                     configuration_index:0x0000
                                reserved:0x0000
                         descriptor_type:ENTITY
                        descriptor_index:0x0000
                                 content: 00 1C AB FF FE 00 05 3A 00 1C AB 90 03 00 30 00 00 00 C5 08 00 06 48 01 00 06 48 01 
00 00 00 00 00 00 84 06 FF FF FF FF FF FF FF FF 6A 69 66 66 79 2D 38 31 36 2D 74 6F 70 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00
32 30 31 34 30 38 30 35 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6A 69 66 66 79 2D 63 75 62 65 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 31 31 31 35 31 32 37 34 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 


```

### avdecc-get-control

```
avdecc-get-control usage:
	avdecc-get-control [verbosity] [timeout_in_ms] [network_port] [sequence_id] [destination_mac] [target_entity_id] [descriptor_index]
```

Example run:

```

```

### avdecc-set-control

```
avdecc-set-control usage:
	avdecc-set-control [verbosity] [timeout_in_ms] [network_port] [sequence_id] [destination_mac] [target_entity_id] [descriptor_index] [payload...]
```

