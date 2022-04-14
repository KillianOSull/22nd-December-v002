#include <zephyr/types.h> //includes a type of library
#include <stddef.h>	  //includes a type of library
#include <string.h>	  //includes a type of library
#include <errno.h> 	  //includes a type of library
#include <sys/printk.h>	  //includes a type of library
#include <sys/byteorder.h>//includes a type of library
#include <zephyr.h>	  //includes a type of library

#include <settings/settings.h>	//includes a type of library

#include <bluetooth/bluetooth.h>//includes a type of library
#include <bluetooth/hci.h>	//includes a type of library
#include <bluetooth/conn.h> 	//includes a type of library
#include <bluetooth/uuid.h>	//includes a type of library
#include <bluetooth/gatt.h>	//includes a type of library
#include <device.h>		//includes a type of library
#include <drivers/sensor.h>	//includes a type of library

#include <stdio.h>		//includes a type of library
		

#include "matrix.h" 		//includes matrix.h file in the directory assignment
#include "buttons.h"		//includes a buttons.h file in the directory assignment

#define BT_UUID_LED_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x3000)		//defines the bluetooth identifier
#define BT_UUID_LED_ID BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x3001)				//defines the bluetooth identifier
#define BT_UUID_BUTTON_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2000)		//defines the bluetooth identifier
#define BT_UUID_BUTTON_A_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2001)		//defines the bluetooth identifier
#define BT_UUID_BUTTON_B_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)24)			//defines the bluetooth identifier

static struct bt_uuid_128 my_led_uuid = BT_UUID_INIT_128( BT_UUID_LED_SERVICE_VAL);		// the 128 bit UUID for this gatt value
static struct bt_uuid_128 led1_id = BT_UUID_INIT_128(BT_UUID_LED_ID); 		   		// the 128 bit UUID for this gatt value

static struct bt_uuid_128 my_button_uuid = BT_UUID_INIT_128( BT_UUID_BUTTON_SERVICE_VAL);	// the 128 bit UUID for this gatt value
static struct bt_uuid_128 button_a_id=BT_UUID_INIT_128(BT_UUID_BUTTON_A_ID); 			// the 128 bit UUID for this gatt value
static struct bt_uuid_128 button_b_id=BT_UUID_INIT_128(BT_UUID_BUTTON_B_ID); 			// the 128 bit UUID for this gatt value

uint32_t led1_value;		// the gatt characateristic value that is being shared over BLE

uint32_t button_a;		// the gatt characateristic value that is being shared over BLE 
uint32_t button_b;		// the gatt characateristic value that is being shared over BLE 
uint32_t button_a_state=0;	// the gatt characateristic value that is being shared over BLE  
uint32_t button_b_state=0;	// the gatt characateristic value that is being shared over BLE 

static ssize_t read_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),	/* specify BLE advertising flags = discoverable, BR/EDR not supported (BLE only) */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LED_SERVICE_VAL 		/* A 128 Service UUID for the our custom service follows */),
};

struct bt_conn *active_conn=NULL; // use this to maintain a reference to the connection with the central device (if any)

static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read on button A state\n");	//prints on commandline when button A is pressed
	const char *value = (const char *)&button_a_state;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_a_state));
}
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read on button B state\n");	//prints on commandline when button B is pressed
	const char *value = (const char *)&button_b_state;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_b_state));
}

BT_GATT_SERVICE_DEFINE(my_button_svc,				//creates a new service for button
	BT_GATT_PRIMARY_SERVICE(&my_button_uuid),
		BT_GATT_CHARACTERISTIC(&button_a_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_button_a, NULL, &button_a_state),		//enables the read feature
		
		BT_GATT_CHARACTERISTIC(&button_b_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_button_b, NULL, &button_b_state)		//enables the read feature
		
);

static ssize_t read_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read to led state %p\n",attr); 	//prints when reading the led state
	matrix_begin();					//function to initilise all LED's on microbit
	// Could use 'const char *value =  attr->user_data' also here if there is the char value is being maintained with the BLE STACK
	const char *value = (const char *)&led1_value; // point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(led1_value)); // pass the value back up through the BLE stack
	
}


BT_GATT_SERVICE_DEFINE(my_led_svc,				//creates a new service for LED
	BT_GATT_PRIMARY_SERVICE(&my_led_uuid),
		BT_GATT_CHARACTERISTIC(&led1_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_led, NULL, &led1_value),		//enables the read and write feature
);

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printf("Connection failed (err 0x%02x)\n", err); //prints if the connection fails
	} else {
		printf("Connected\n");
		active_conn = conn;				//puts conn into active_conn when connected
	}
}
// Callback that is activated when a connection with a central device is taken down
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	active_conn = NULL;					//puts NULL into active_conn when disconnected
}
// structure used to pass connection callback handlers to the BLE stack
static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};
// This is called when the BLE stack has finished initializing
static void bt_ready(void)
{
	int err; 				//local veriable called err
	printf("Bluetooth initialized\n"); 	//prints when br_ready function is called
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printf("Advertising failed to start (err %d)\n", err);
		return;
	} //prints if there a fault for advertising
	printf("Advertising successfully started\n");
}

void main(void)				//main
{	
	unsigned int lock_code = 0b00110110100101001010010111010010;
	unsigned int unlock_code = ;
	int err;			//new variable err
	int counter = 0;
	uint8_t rows = 1;		//initilsing rows for led matrix 
	uint8_t cols = 1;		//initilsing cols for led matrix
		//calling lsm303_ll_begin function in lsm303_ll.c
	err = buttons_begin();	//calling buttons begin function in buttons.c
	if (err < 0)
	{
		printf("\nError initializing buttons.  Error code = %d\n",err);	
	 while(1);
	} //error checking for buttons
	err = matrix_begin();
	if (err < 0)
	{
		printf("\nError initializing buttons.  Error code = %d\n",err);	
	 while(1);
	}//error checking for led matrix
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}	//error checking for bluetooth
	bt_ready(); // This function starts advertising
	bt_conn_cb_register(&conn_callbacks);
	printf("Zephyr Microbit V2 minimal BLE example! %s\n", CONFIG_BOARD);			
	while (1) {
	
		//k_sleep(K_SECONDS(0.2));				//sleeps for 1 second
		matrix_all_off();
			if(get_buttonA() == 0){			
				printf("\n Button A pressed!\n");	// prints when button A has been pressed
				counter++;
				printf("Counter value: %d\n", counter);
				printf("UNLOCK CODE: %i\n", unlock_code);

				rows =  0b11111;		// sets the rows to all on		
           		cols =  0b10000; 		// sets the cols to all off
           		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           		k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b01000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00100; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00010; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00001; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00010; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00100; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b01000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b10000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b01000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00100; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00010; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00001; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00010; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b00100; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b01000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b11111;		// sets the rows to all on		
           			cols =  0b10000; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));

           			matrix_all_off();
           			k_sleep(K_SECONDS(1));
           			
				}
			if(get_buttonB() == 0)			
			{
				counter = 0;
				printf("\n Button B pressed!\n");	//prints when button B has been pressed
				printf("Counter value: %d\n", counter);
				printf("LOCK CODE: %i\n", lock_code);
				rows =  0b10000;		// sets the rows to all on		
           		cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b01000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00100;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00010;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00001;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00010;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00100;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b01000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b10000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b01000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00100;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00010;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00001;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00010;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b00100;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b01000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));
           			
           			rows =  0b10000;		// sets the rows to all on		
           			cols =  0b11111; 		// sets the cols to all off
           			matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
           			k_sleep(K_SECONDS(0.05));

           			matrix_all_off();
           			k_sleep(K_SECONDS(1));
			}
		if (active_conn)				//if there is an active connection, notify the stepcount, button A and Button B
		{
		bt_gatt_notify(active_conn,&my_button_svc.attrs[2], &button_a_state,sizeof(button_a_state));
		bt_gatt_notify(active_conn,&my_button_svc.attrs[3], &button_b_state,sizeof(button_b_state));		
		}
			
	}
}
