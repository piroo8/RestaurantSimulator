#include "a1.h"
#include <stdio.h>
#include <string.h>
#define MAX_LINE_LEN 126 //includes termination char

int item_count(char* fname){
	FILE *open_file = fopen(fname, "r");
	int count = 0;
	char c;

	for (c = getc(open_file); c != EOF; c = getc(open_file)){
		if (c == '$'){
            count++;
        }
	}
	fclose(open_file);
	return count;

}

Menu* load_menu(char* fname){

    Menu *ptr_menu = (Menu *)malloc(sizeof(Menu));
    int i = 0;
	char line[MAX_LINE_LEN];
    int count = 0;

	// get count of items
    count = item_count(fname);
    ptr_menu->num_items = count;
    
    char multi_line[count][MAX_LINE_LEN];
    FILE *open_file = fopen(fname, "r");

    char dollar = '$';
    
	while(i<count){
        int ok_to_add = 0;
		fgets(line, MAX_LINE_LEN, open_file);
        // printf("line is: %s",line);
            for (int z = 0; z <= MAX_LINE_LEN; z++){
                // printf("%c",line[z]);
                if (line[z] == dollar){
                    
                    // printf("ok to add: %d", i);
                    ok_to_add = 1;
                    break;
                }
            }
        if (ok_to_add){
            // printf("ok to add");
            strcpy(multi_line[i],line);
            // printf("the chars input were %s\n", line);
            // printf("Multi at i: %d is now is now: %s\n", i, multi_line[i]);
            i++;
        }

	}
    rewind(open_file);
	fclose(open_file);
    double item_prices[count];

    ptr_menu->item_codes = (char **)malloc(sizeof(char*) * count);
    ptr_menu->item_names = (char **)malloc(sizeof(char*) * count);
    ptr_menu->item_cost_per_unit = (double *)malloc(sizeof(double) * count);

    for (int j = 0; j < count; j++){
        //allocate space for the item code strings
        (ptr_menu->item_codes)[j] = (char *)malloc(sizeof(char) * ITEM_CODE_LENGTH);
        
        //allocate space for item_name strings
        (ptr_menu->item_names)[j] = (char *)malloc(sizeof(char) * MAX_ITEM_NAME_LENGTH);
    }

    char* price = "";
    for (int k = 0; k < count; k++){
        //put item code into the allocated space
        strcpy((ptr_menu->item_codes)[k], strtok(multi_line[k], MENU_DELIM));

        //put item name into space
        strcpy((ptr_menu->item_names)[k], strtok(NULL, MENU_DELIM));

        price = strtok(NULL, MENU_DELIM);

        //put price into space
        (ptr_menu->item_cost_per_unit)[k] = strtod((price + 1), NULL);
    }
    return ptr_menu;

}

Order* build_order(char *items ,char *quantities ){
    //declarations
    char *order_items = items;
    int character_amt = strlen(items);
    int item_amt = character_amt/(ITEM_CODE_LENGTH - 1);
    int quant_char_amt = strlen(quantities);

    //put items into an array. strtok only works with array not string literals
    char quant_arr[quant_char_amt];
    strcpy(quant_arr, quantities);

    //create order and allocate memory for the pointers
    Order *new_order = (Order *)malloc(sizeof(Order));
    new_order->num_items = item_amt;
    new_order->item_codes = (char**)malloc(sizeof(char*) * item_amt);
    new_order->item_quantities = (int*)malloc(sizeof(int) * item_amt);

    //allocate space for code strings
    for (int i = 0; i < item_amt; i++){
        (new_order->item_codes)[i] = (char *)malloc(sizeof(char) * ITEM_CODE_LENGTH);
    }
    
    //put item codes in
    char* finished_arr[character_amt];
    char sub_str[3];

    for (int j = 0; j < character_amt; j += 2){
        strncpy(sub_str, items + j, 2);
        sub_str[2] = '\0';
        strcpy((new_order->item_codes)[j/2],sub_str);
    }

    //put the quantities into the order
    for (int k = 0; k < item_amt; k++){
        if (k == 0){
            (new_order->item_quantities)[k] = atoi(strtok(quant_arr,MENU_DELIM));
        }
        else{
            (new_order->item_quantities)[k] = atoi(strtok(NULL,MENU_DELIM));
        }
    }
    return new_order;
}

double get_item_cost ( char * item_code , Menu * menu ){
    for (int i = 0; i < (menu->num_items); i++){
        if (!strcmp(menu->item_codes[i], item_code)){
            return (menu->item_cost_per_unit)[i];
        }
    }
}

double get_order_subtotal(Order* order , Menu* menu){
    double sum = 0;
    for ( int i = 0; i < (order->num_items); i++){
        sum += order->item_quantities[i] * get_item_cost((order->item_codes[i]),menu);
    }
    return sum;
}

double get_order_total(Order* order, Menu* menu){
    return (1+0.01*TAX_RATE) * get_order_subtotal(order, menu);
}

int get_num_completed_orders(Restaurant* restaurant){
    return restaurant->num_completed_orders;
}

int get_num_pending_orders(Restaurant* restaurant){
    return restaurant->num_pending_orders;
}

void clear_order(Order** order){
    free((*order)->item_quantities);
    for (int i = 0; i <(*order)->num_items; i++){
        free((*order)->item_codes[i]);
    }
    free((*order)->item_codes);
    free(*order);
    *order = NULL;
}

void clear_menu(Menu** menu){
    for (int i = 0; i <(*menu)->num_items; i++){
        free((*menu)->item_codes[i]);
        free((*menu)->item_names[i]);
    }
    free((*menu)->item_codes);
    free((*menu)->item_names);
    free((*menu)->item_cost_per_unit);
    free(*menu);
}

void print_menu(Menu* menu){
	fprintf(stdout, "--- Menu ---\n");
	for (int i = 0; i < menu->num_items; i++){
		fprintf(stdout, "(%s) %s: %.2f\n", 
			menu->item_codes[i], 
			menu->item_names[i], 
			menu->item_cost_per_unit[i]	
		);
	}
}


void print_order(Order* order){
	for (int i = 0; i < order->num_items; i++){
		fprintf(
			stdout, 
			"%d x (%s)\n", 
			order->item_quantities[i], 
			order->item_codes[i]
		);
	}
}

Restaurant * initialize_restaurant ( char *name ){
    
    Restaurant* new_res = (Restaurant* )malloc(sizeof(Restaurant));
    
    new_res->name = name;
    
    new_res->menu = load_menu(MENU_FNAME);
    
    new_res->num_completed_orders = 0;
    
    new_res->num_pending_orders = 0;

    //initialize queue
    new_res->pending_orders = (Queue *)malloc(sizeof(Queue));
    new_res->pending_orders->head = NULL;
    
    new_res->pending_orders->tail = NULL;
    return new_res;
}

void enqueue_order(Order *order, Restaurant * restaurant){
    QueueNode *new_q = (QueueNode* )malloc(sizeof(QueueNode));
    new_q->order = (Order *)malloc(sizeof(Order));
    new_q->order = order;

    new_q->next = NULL;

    //Enqueing First Node to empty list
    if ((restaurant->pending_orders->head) == NULL){

        restaurant->pending_orders->head = new_q;
        restaurant->pending_orders->tail = new_q;

    }
    else{
        restaurant->pending_orders->tail->next = new_q;
        restaurant->pending_orders->tail = new_q;
    }
    (restaurant->num_pending_orders)++;
}

Order* dequeue_order(Restaurant * restaurant){
    Order *return_order;

    return_order = restaurant->pending_orders->head->order;

    struct QueueNode *bye_node = restaurant->pending_orders->head;
    restaurant->pending_orders->head = restaurant->pending_orders->head->next; 
    free(bye_node);
    
    //If the last node was removed
    if((restaurant->pending_orders->head)==NULL){
        restaurant->pending_orders->tail = NULL;
    }

    (restaurant->num_pending_orders)--;
    (restaurant->num_completed_orders)++;

    return return_order;
}

void close_restaurant(Restaurant** restaurant){
    clear_menu(&((*restaurant)->menu));

    for (int i = 0; i <(*restaurant)->num_pending_orders; i++){
        Order* temp_order = dequeue_order(*restaurant);
        clear_order(&temp_order);
    }
    free((*restaurant)->pending_orders);

    free(*restaurant);
    *restaurant = NULL;
}


void print_receipt(Order* order, Menu* menu){
	for (int i = 0; i < order->num_items; i++){
		double item_cost = get_item_cost(order->item_codes[i], menu);
		fprintf(
			stdout, 
			"%d x (%s)\n @$%.2f ea \t %.2f\n", 
			order->item_quantities[i],
			order->item_codes[i], 
			item_cost,
			item_cost * order->item_quantities[i]
		);
	}
	double order_subtotal = get_order_subtotal(order, menu);
	double order_total = get_order_total(order, menu);
	
	fprintf(stdout, "Subtotal: \t %.2f\n", order_subtotal);
	fprintf(stdout, "               -------\n");
	fprintf(stdout, "Tax %d%%: \t$%.2f\n", TAX_RATE, order_total);
	fprintf(stdout, "              ========\n");
}