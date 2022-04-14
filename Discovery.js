const deviceOfInterest = 'CC:0C:27:E4:90:EC'									//mac address of device

const buttonServiceOfInterestUuid = '00000001-0002-0003-0004-000000002000' 					//uuid of button service
const buttonCharacteristicOfInterestUuid = '00000001-0002-0003-0004-000000002001' 				//uuid of read/notify characteristic of button A service
const buttonCharacteristic2OfInterestUuid = '00000001-0002-0003-0004-000000002002' 				//uuid of read/notify characteristic of button B service

const ledServiceOfInterestUuid = '00000001-0002-0003-0004-000000003000' 					//uuid of LED service
const ledCharacteristicOfInterestUuid = '00000001-0002-0003-0004-000000003001' 					//uuid of read/write characteristic of LED service

var buttonChar;													//variable actuatorChat
var ledChar;													//variable notifychar

/*
if (services.includes(actuatorServiceOfInterestUuid)) { 							//uuid of LED service
  	console.log('got the LED service')									//prints on command line
  	const primaryLedService = await gattServer.getPrimaryService(ledServiceOfInterestUuid)	
	ledChar = await primaryLedService.getCharacteristic(ledCharacteristicOfInterestUuid)  		//uuid of read/write characteristic of LED service
}
if (services.includes(serviceOfInterestUuid)) { 								//uuid of accel service
	console.log('got the Accel service')									//prints on command line
	
}														
if (services.includes(notifyServiceOfInterestUuid)) { 								//uuid of notify service
	console.log('got the button service')
	
}
*/

const main = async() => { 											//main function

   while(1){
														//async function main () {
  	const {createBluetooth}=require('node-ble') 								//nodejs ble module/library
  	const { bluetooth, destroy } = createBluetooth()							//get bluetooth adapter
  	const adapter = await bluetooth.defaultAdapter() 							//get an available Bluetooth adapter
  	if(!await adapter.isDiscovering()){
  	   await adapter.startDiscovery() 										//using the adapter, start a device discovery session  
  	}
  	console.log('discovering')										//prints on command line
  	
  	const device = await adapter.waitDevice(deviceOfInterest)						//usese devices specified mac addess from top of programme
  	console.log('got device', await device.getAddress())							//await device.getAddress())
  	const deviceName = await device.getName()								//gets the name of the device
  	console.log('got device remote name', deviceName)							//prints on command line
  	console.log('got device user friendly name', await device.toString())					//prints on command line
  	console.log('Device: [',deviceName, '] is within range')

  	await adapter.stopDiscovery() 										//stops looking for devices
  														//connect to the specific device
  	await device.connect()											//connects to specified device
  	if(device.connect() == 'true'){
  	   console.log("connected to device : " + deviceName)
  	}							//prints on command line
  	
  	await new Promise(resolve => setTimeout(resolve, 10000))						//waits 1 min
  	
  	await device.disconnect()
  	destroy()
  	console.log('disconnected')
  	
   } 	
  	
}  	
main()
  .then()
  .catch(console.error)
