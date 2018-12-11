# device-signature
API and modules for accessing device signature data.


Component serial number
Signature does not specify what format the serial number should take, but
the recommended approach is to first test if the serial number is an UUID, if
it is not, then see if it is printable (ASCII) and finally if everything else
fails, then just encode it as HEX.
