# ESP magnet

ESP32 firmware to trigger the electromagnet.

## Connection

`GPIO_4` <--> `DOUT_PIN`

## Usage

Connect the serial port of ESP32 dev board to a computer, then create a serial connection with following configuration

| Item     | Value  |
| -------- | ------ |
| Baudrate | 115200 |
| Stop     | 1      |
| Parity   | None   |
| Control  | None   |

After the establishment of connection, use following command to control the board

| Format        | Function                    |
|---------------|-----------------------------|
| `enable`      | start the magnet            |
| `is_enabled`  | judge if magnet is enabled  |
| `disable`     | trigger for once            |
| `is_disabled` | judge if magnet is disabled |
| `restart`     | restart                     |
| `get_status`  | check the status            |


