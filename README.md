# Warden-Defender
###Door controller software for Warden

-----
The Warden Defender is the hardware/software implimentation for the Warden Manager. The basic idea behind the Warden Defender is an open source door controller for RIFD keyless entry. There will be a custom interface board to allow for easy setup just plugging in the reader and strike then connecting it to the arduino.

The system will send a request to the server to ask for access, if the card ID matches an ID in the database and the user is eabled the server will send the unlock signal and the Defender will open the door.

## TO DO:
- Basics
 - [ ] HID Prox reader interface
 - [ ] Electronic Strike interface
 - [ ] Network Interface
- Hardware
 - [ ] Design custom reader/strike interface board
 - [ ] POE Compatable somehow
 - [ ] Get custom board printed for testing
- Software
 - [ ] Network connectivity to server
 - [ ] Open strike from network without swiping card
 - [ ] offline access list?
 - [ ] management web page for board?
 - [ ] setup different loops for manage/control webpage and reader 


##Best Practices for setup:

##Links to hardware:
