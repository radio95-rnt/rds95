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

## Disclaimer

RDS95 is based on [Anthony96922](https://github.com/Anthony96922/)'s [MiniRDS](https://github.com/Anthony96922/MiniRDS)
