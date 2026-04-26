# RDS95

(eRT)(RDS2)

RDS95 is a light software RDS encoder for linux

RDS95 also embeds Lua to implement some things

RDS95 follows the IEC 62106 standard (available at the RDS Forum website)

Also, yes i would like to license this under the unlicese but the tyranny of the GPL license restricts me from doing that, this is a message to you, anthony (see the [disclaimer](#disclaimer))

## Diffrences between IEC 62106 and EN 50067

The newer standard which is the IEC one, removes these features:

- MS
- PIN
- LIC
- DI (partially, only dynamic pty is left)
- EWS (now ODA)
- IH (now ODA)
- RP
- TDC (now ODA)

## Unique features

RDS95 is the only (as far as i can tell) encoder to transmit the 9-bit AF codes

## Lua engine

Yet another unique feature, the Lua engine allows you to control the encoder, almost entirely.

Every RDS2 subcarrier is in full control of Lua (except tunneling - core fills in PTY and the rest)

### RFT

For a long time there was no RFT in RDS95, but on 28th December 2025 that changed when i implemented it in lua, just call the load function with the path and id 0, and see the your station's logo in the decoder

And yes, it is implemented in only Lua and the ODA engine which is also in Lua

## Disclaimer

RDS95 is "based" on [Anthony96922](https://github.com/Anthony96922/)'s [MiniRDS](https://github.com/Anthony96922/MiniRDS) licensed under GPLv3
