#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFERSIZE 512
#define MAX_YR 9999
#define MIN_YR 2000

typedef struct
{
    int yyyy;
    int mm;
    int dd;
} Date;

typedef struct
{
  char *flt_line;
} flight_lines;

// structure to save the flight details
typedef struct FL_details
{
    char *FLight_id;
    char *Timing;
    char *Timing2;
    int seats_available_e;
    int total_seats_e;
    float fare_economy;
    int seats_available_b;
    int total_seats_b;
    float fare_business;
} flight_details;

// structure to save the traveller details.
typedef struct TRaveller
{
    char *name;
    int age;
    char *fl_id;
    char *class;
    float conv_seat_price;
    float seat_price;
    char *source;
    char *destination;
    char *departure;
    char *arrival;
    Date getDate;
    long int mobile_no;
} traveller_details;

char* receiveMsgFromServer(int sock_fd) {
    int rbuf = 0;
    int n = read(sock_fd, &rbuf, sizeof(int));
    if(n <= 0) {
        shutdown(sock_fd, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(rbuf*BUFFERSIZE);
    memset(str, 0, rbuf*BUFFERSIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < rbuf; ++i) {
        int n = read(sock_fd, str, BUFFERSIZE);
        str = str+BUFFERSIZE;
    }
    return str_p;
}

void sendMsgToServer(int sock_fd, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(sock_fd, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(sock_fd, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}
int airport_valid(char *airplane_name)
{
    char ports[5][50] = {"Tokyo", "Sapporo", "Naha", "Fukuoka", "Osaka"};
    int ctr = 0;
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(airplane_name, ports[i]) == 0)
        {
            ctr = 1;
        }
    }
    return ctr;
}

int fm(int date, int month, int year)
{
    int fmonth, leap;
    if ((year % 100 == 0) && (year % 400 != 0))
        leap = 0;
    else if (year % 4 == 0)
        leap = 1;
    else
        leap = 0;
    fmonth = 3 + (2 - leap) * ((month + 2) / (2 * month)) + (5 * month + month / 9) / 2;
    fmonth = fmonth % 7;
    return fmonth;
}

//function that converts the date to day.
char *day_of_week(int date, int month, int year)
{
    int dayOfWeek;
    int YY = year % 100;
    int century = year / 100;
    char *day = malloc(20);
    printf("\nDate: %d/%d/%d \n", date, month, year);

    dayOfWeek = 1.25 * YY + fm(date, month, year) + date - 2 * (century % 4);

    dayOfWeek = dayOfWeek % 7;
    if (dayOfWeek == 0)
        strcpy(day, "Saturday\n");
    else if (dayOfWeek == 1)
        strcpy(day, "Sunday\n");
    else if (dayOfWeek == 2)
        strcpy(day, "Monday\n");
    else if (dayOfWeek == 3)
        strcpy(day, "Tuesday\n");
    else if (dayOfWeek == 4)
        strcpy(day, "Wednesday\n");
    else if (dayOfWeek == 5)
        strcpy(day, "Thursday\n");
    else if (dayOfWeek == 6)
        strcpy(day, "Friday\n");
    else
        strcpy(day, "Invalid Date");
    return day;
}

int IsLeapYear(int year)
{
    return (((year % 4 == 0) &&
             (year % 100 != 0)) ||
            (year % 400 == 0));
}
// returns true i.e. 1, if given date is valid.
int isValidDate(Date *validDate)
{
    time_t timing = time(NULL);
    struct tm tm = *localtime(&timing);
    if (validDate->yyyy > MAX_YR ||
        validDate->yyyy < (tm.tm_year + 1900)) //if the year that you have enter is less than the minim year or greater than the max year than its a invalid year
        return 0;
    if (validDate->mm < (tm.tm_mon + 1) || validDate->mm > 12) //similarly no of month is basically 12 hence the enter number should not be more than 12 or less than 1
        return 0;
    if (validDate->dd < tm.tm_mday || validDate->dd > 31)
        return 0;
    //Handle feb days in leap year
    if (validDate->mm == 2) // as we know that leap year has 29 days in feb in constrast to normal year which has only 28 days
    {
        if (IsLeapYear(validDate->yyyy))
            return (validDate->dd <= 29); //leap year has 29 days in feb
        else
            return (validDate->dd <= 28);
    }
    if (validDate->mm == 4 || validDate->mm == 6 ||
        validDate->mm == 9 || validDate->mm == 11) // respective month april, June, september and november has 30 days
        return (validDate->dd <= 30);
    return 1;
}
char *Day(Date *getDate)
{

    int status = 0;
    char *day_string = day_of_week(getDate->dd, getDate->mm, getDate->yyyy);
    printf("You journey is on = %s\n", day_string);
    return day_string;
}

// price conversion for economy class
// 75% discount for infants, 50% for childs, and 0% for adults.
float convert_e(int *age, float *price)
{
    float new_price;
    if (*age < 5)
    { new_price = 0.25 * (*price);  }
    else if (*age < 18 && *age >= 5)
    {  new_price = 0.5 * (*price);}
    else
    { new_price = *price;  }
    return new_price;
}
// price conversion for economy class
// 60% discount for infants, 35% for childs, and 0% for adults.
float convert_b(int *age, float *price)
{
    float new_price;
    if (*age < 5)
    { new_price = 0.40 * (*price);}
    else if (*age < 18 && *age >= 5)
    { new_price = 0.65 * (*price);  }
    else
    {  new_price = *price;}
    return new_price;
}

void bookTickets(FILE *fp_orig, FILE *fp_mod, FILE *ticket)
{
    Date getDate = {0};
    printf("\nEnter your journey date in format (dd/mm/yyyy): "); // input from the user in dd/mm/yy
    scanf("%d/%d/%d", &getDate.dd, &getDate.mm, &getDate.yyyy);
    char *day_of_travel = Day(&getDate);

    printf("Available airports: Tokyo, Sapporo, Naha, Fukuoka, Osaka\n");
    char buffer[100];
    char *board = malloc(sizeof(char) * 30);
    char *dest = malloc(sizeof(char) * 30);
    // enter your source airport.
    printf("Enter your source of Journey : ");
    scanf("%s", board);
    printf("Enter your destination of Journey : ");
    scanf("%s", dest);
    if (!(airport_valid(board) && airport_valid(dest)))
    {
        printf("Please enter a valid airport name!:|\n");
        bookTickets(fp_orig, fp_mod, ticket);
    }

    while (fgets(buffer, 100, fp_orig) != NULL)
    {
        if (strcmp(day_of_travel, buffer) == 0)
        {
            fputs(buffer, fp_mod);
            char board_to_dest[100], board_dest[100];
            fgets(board_to_dest, 100, fp_orig);
            sprintf(board_dest, "%s to %s\n", board, dest);

            if (strcmp(board_to_dest, board_dest) == 0)
            {
                fputs(board_to_dest, fp_mod);
                flight_lines *fl_line = malloc(sizeof(flight_lines) * 6);
                printf("\n\nAvailable flights on %s\n", day_of_travel);
                printf("\n======================================================\n");
                printf("\nFlit-ID Dept. Arrv. Eco Ect EcoCost Bus But BusCost\n");
                printf("------------------------------------------------------\n");
                for (int i = 0; i < 6; i++)
                {
                    char *flight_line = malloc(sizeof(char) * (100));
                    fgets(flight_line, sizeof(char) * 100, fp_orig);
                    printf("* %s", flight_line);
                    fl_line[i].flt_line = flight_line;
                }
                printf("\n======================================================\n");
                printf("\n");
                printf("Do you want to go further? y/n\n");
                char Bt[1];
                scanf("%s", Bt);
                if (strcmp(Bt, "y") == 0)
                {
                    printf("Choose your Plane by its Id\n");
                    char Id[5];
                    scanf("%s", Id);
                    flight_details *F_details = malloc(sizeof(flight_details) * 6);
                    for (int i = 0; i < 6; i++)
                    {
                        char *flight_name = malloc(sizeof(char) * (5 + 1));
                        char *timing = malloc(sizeof(char) * (5 + 1));
                        char *timing2 = malloc(sizeof(char) * (5 + 1));
                        flight_name[5] = '\0';
                        timing[5] = '\0';
                        timing2[5] = '\0';
                        int eco_seats_avail, tot_eco, bus_seats_avail, tot_bus;
                        float eco_fare, bus_fare;

                        sscanf(fl_line[i].flt_line, "%s %s %s %d %d %f %d %d %f", flight_name, timing, timing2, &eco_seats_avail,
                               &tot_eco, &eco_fare, &bus_seats_avail, &tot_bus, &bus_fare);
                        F_details[i].FLight_id = flight_name;
                        F_details[i].Timing = timing;
                        F_details[i].Timing2 = timing2;
                        F_details[i].seats_available_e = eco_seats_avail;
                        F_details[i].total_seats_e = tot_eco;
                        F_details[i].fare_economy = eco_fare;
                        F_details[i].seats_available_b = bus_seats_avail;
                        F_details[i].total_seats_b = tot_bus;
                        F_details[i].fare_business = bus_fare;

                        if (strcmp(Id, F_details[i].FLight_id) == 0)
                        {
                            printf("Enter no. of Travellers : ");
                            int pass_no;
                            scanf("%d", &pass_no);
                            traveller_details *Ticket = malloc(sizeof(traveller_details) * pass_no);
                            float price = 0.0;
                            int bus_ctr = 0, eco_ctr = 0;

                            for (int j = 0; j < pass_no; j++)
                            {
                                printf("\nEnter details for traveller no. %d : \n", (j + 1));
                                printf("\nEnter your last name ONLY : \n");
                                char *traveller_name = malloc(sizeof(char) * 50);
                                scanf("%s", traveller_name);
                                Ticket[j].name = traveller_name;
                                printf("\nEnter your age : \n");
                                int age_name;
                                scanf("%d", &age_name);
                                Ticket[j].age = age_name;
                                printf("\nEnter your Mobile Number : \n");
                                long int mobile;
                                scanf("%ld", &mobile);
                                Ticket[j].mobile_no = mobile;
                                char BE[1];
                                printf("\nIn which class '%s' wanna travel? Economy class or Business class.\n", Ticket[j].name);
                                printf("Enter 'B' for Business Class and 'E' for Economy Class -- \n");
                                scanf("%s", BE);
                                if (strcmp(BE, "B") == 0)
                                {
                                    Ticket[j].class = "Business";
                                    Ticket[j].seat_price = F_details[i].fare_business;
                                    Ticket[j].conv_seat_price = convert_b(&Ticket[j].age, &Ticket[j].seat_price);
                                    price += Ticket[j].conv_seat_price;
                                    F_details[i].seats_available_b -= 1;
                                    bus_ctr++;
                                }
                                else
                                {
                                    Ticket[j].class = "Economy";
                                    Ticket[j].seat_price = F_details[i].fare_economy;
                                    Ticket[j].conv_seat_price = convert_e(&Ticket[j].age, &Ticket[j].seat_price);
                                    price += Ticket[j].conv_seat_price;
                                    F_details[i].seats_available_e -= 1;
                                    eco_ctr++;
                                }
                                //sscanf(board_to_dest, "%s %*s %s", Ticket[j].source, Ticket[j].destination);
                                Ticket[j].fl_id = F_details[i].FLight_id;
                                Ticket[j].source = board;
                                Ticket[j].destination = dest;
                                Ticket[j].departure = F_details[i].Timing;
                                Ticket[j].arrival = F_details[i].Timing2;
                                Ticket[j].getDate.dd = getDate.dd;
                                Ticket[j].getDate.mm = getDate.mm;
                                Ticket[j].getDate.yyyy = getDate.yyyy;
                            }
                            printf("Do you want to checkout? 0/1\n");
                            printf("Enter 1 to checkout otherwise enter 0\n");
                            int YN;
                            scanf("%d", &YN);
                            if (YN)
                            {
                                ticket = fopen("Ticket.txt", "w");
                                if (ticket == NULL)
                                {
                                    printf("\nUnable to open Ticket.txt file.\n");
                                    printf("Please check whether file exists and you have write privilege.\n");
                                    exit(EXIT_FAILURE);
                                }
                                fprintf(ticket, "TICKET\n");
                                fprintf(ticket, "=============================================================\n");
                                for (int j = 0; j < pass_no; j++)
                                {
                                    fprintf(ticket, "Name : %s\nAge :%d\nFlight ID : %s\nClass : %s\nFare : %.1f\nSource : %s\
                                    \nDestination : %s\nDeparture time : %s\nArrival time : %s\
                                    \nDate of Boarding : %02d/%02d/%d\nMobile No.: %ld\n\n\n",
                                            Ticket[j].name, Ticket[j].age, Ticket[j].fl_id,
                                            Ticket[j].class, Ticket[j].conv_seat_price, Ticket[j].source, Ticket[j].destination,
                                            Ticket[j].departure, Ticket[j].arrival, Ticket[j].getDate.dd, Ticket[j].getDate.mm,
                                            Ticket[j].getDate.yyyy, Ticket[j].mobile_no);
                                    fprintf(ticket, "\n---------------------------------------------------------\n");
                                }
                                fprintf(ticket, "\n\n=============================================================\n\n");
                                fprintf(ticket, "Total Fare : %.1f\n\n\n", price);
                                fprintf(ticket, "=============================================================\n");
                                printf("Your Bill:\n");
                                printf("=============================================================\n");
                                for (int j = 0; j < pass_no; j++)
                                {
                                    printf("Name : %s\nAge :%d\nFlight ID : %s\nClass : %s\nFare : %.1f\nSource : %s\
                                    \nDestination : %s\nDeparture time : %s\nArrival time : %s\
                                    \nDate of Boarding : %d/%d/%d\nMobile No.: %ld\n",
                                           Ticket[j].name, Ticket[j].age, Ticket[j].fl_id,
                                           Ticket[j].class, Ticket[j].conv_seat_price, Ticket[j].source, Ticket[j].destination,
                                           Ticket[j].departure, Ticket[j].arrival, Ticket[j].getDate.dd, Ticket[j].getDate.mm,
                                           Ticket[j].getDate.yyyy, Ticket[j].mobile_no);
                                    printf("\n---------------------------------------------------------\n");
                                }

                                printf("\n\n=============================================================\n\n");
                                printf("Total Fare : %.1f\n\n", price);
                                printf("=============================================================\n");
                                //free(Ticket);
                                printf("\n\nYOUR TICKET BOOKED SUCCESSFULLY !\n");
                                printf("HAPPY JOURNEY ! :)\n\n\n");
                                fprintf(fp_mod, "%s %s %s %d %d %.1f %d %d %.1f\n", F_details[i].FLight_id, F_details[i].Timing,
                                        F_details[i].Timing2, F_details[i].seats_available_e, F_details[i].total_seats_e, F_details[i].fare_economy,
                                        F_details[i].seats_available_b, F_details[i].total_seats_b, F_details[i].fare_business);
                                fclose(ticket);
                            }
                            else
                            {
                                F_details[i].seats_available_e += eco_ctr;
                                F_details[i].seats_available_b += bus_ctr;

                                fprintf(fp_mod, "%s %s %s %d %d %.1f %d %d %.1f\n", F_details[i].FLight_id, F_details[i].Timing,
                                        F_details[i].Timing2, F_details[i].seats_available_e, F_details[i].total_seats_e, F_details[i].fare_economy,
                                        F_details[i].seats_available_b, F_details[i].total_seats_b, F_details[i].fare_business);
                                printf("Do you want to book again? y/n\n");
                                char Bool[1];
                                scanf("%s", Bool);
                                if (strcmp(Bool, "y") == 0)
                                {
                                    bookTickets(fp_orig, fp_mod, ticket);
                                }
                                else
                                {
                                    printf("Thank you for booking ticket with us\n");
                                    printf("Hope to see you again! :)\n");
                                }
                            }
                        }
                        else
                        {
                            fputs(fl_line[i].flt_line, fp_mod);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 6; i++)
                    {
                        fputs(fl_line[i].flt_line, fp_mod);
                    }
                    printf("Hope you enjoyed our booking service!\n");
                    printf("Hoping to meet you again...\n");
                    //continue;
                }
            }
            else
            {
                fputs(board_to_dest, fp_mod);
            }
        }
        else
        {
            fputs(buffer, fp_mod);
        }
    }
}


void sendMsgToServer(int sock_fd, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(sock_fd, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(sock_fd, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}
    while(1) {
        msgFromServer = receiveMsgFromServer(sock_fd);
        if(msgFromServer == NULL)
            break;
        printf("%s\n",msgFromServer);
        free(msgFromServer);
    }
    printf("Write end closed by the server.\n");
    shutdown(sock_fd, SHUT_RD);
    printf("Connection closed gracefully.\n");
    return 0;


}
