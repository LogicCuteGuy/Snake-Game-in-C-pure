char maker[] = "LogicCuteGuy";

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <math.h>

#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 256
#define MAX_ENTRIES 100

typedef struct
{
    int x, y;
} Vector;

typedef struct
{
    unsigned int player;
    Vector pos;
    Vector speed;
    Vector *tail;
    Vector input;
    unsigned length;
} Snake;

typedef struct
{
    Vector pos;
} Food;

typedef struct
{
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
} ConfigEntry;

typedef struct
{
    ConfigEntry entries[MAX_ENTRIES];
    int count;
} ConfigFile;

unsigned int choiceSe = 0;
unsigned int *choiceValInt;
bool choiceValSe = false;

bool player2 = false;

unsigned int fps = 15;
unsigned int **pix;
bool lp = true;
unsigned int height = 15;
unsigned int width = 15;
unsigned updateCount;
unsigned delay = 0;
unsigned int foods = 1;

// score max
unsigned int max_tail = 0;

COORD cur = {0, 0};
char buf[1024];

Snake *snk1;
Snake *snk2;
Food *food[10];

ConfigFile config;
char filename[] = "snake.config";

void game_setup();
void game_loop();
DWORD WINAPI game_keyboard(void *data);
void game_draw();
void reset_pixels();
void frameRate(unsigned);
void snake_update(Snake *snk);
void snake_draw(Snake *snk);
void snake_eat(Snake *snk);
void snake_die(Snake *snk);
void snake_check_die(Snake *snk1, Snake *snk2);
void snake_set_move(Snake *snk, int x, int y);
void snake_check_food(Snake *snk, Food *fd[10]);
Snake *snake_create();
void snake_destroy(Snake *snk);
Food *food_create();
void food_draw(Food *fd[10]);
void food_destroy(Food *fd);
void setting_page();

// Utility function for case-insensitive string comparison
int strcasecmp(const char *s1, const char *s2);

// Stringify functions
char *stringify_int(int value);
char *stringify_float(float value);
char *stringify_bool(bool value);

// Parsing functions
int parse_int(const char *str);
float parse_float(const char *str);
bool parse_bool(const char *str);

// Functions to read and write config files
void write_config(const char *filename, ConfigFile *config);
void read_config(const char *filename, ConfigFile *config);

// Helper functions to get, set, and delete values by key
const char *get_config_value_by_key(const ConfigFile *config, const char *key);
void set_config_value_by_key(ConfigFile *config, const char *key, const char *value);
void delete_config_entry_by_key(ConfigFile *config, const char *key);

int main()
{
    system("cls");
    read_config(filename, &config);
    if (config.count == 0)
    {
        set_config_value_by_key(&config, "2player", stringify_bool(player2));
        set_config_value_by_key(&config, "foods", stringify_int(foods));
        set_config_value_by_key(&config, "max_tail", stringify_int(max_tail));
        set_config_value_by_key(&config, "fps", stringify_int(fps));
        set_config_value_by_key(&config, "height", stringify_int(height));
        set_config_value_by_key(&config, "width", stringify_int(width));
        write_config(filename, &config);
    }
    else
    {
        player2 = parse_bool(get_config_value_by_key(&config, "2player"));
        foods = parse_int(get_config_value_by_key(&config, "foods"));
        max_tail = parse_int(get_config_value_by_key(&config, "max_tail"));
        fps = parse_int(get_config_value_by_key(&config, "fps"));
        height = parse_int(get_config_value_by_key(&config, "height"));
        width = parse_int(get_config_value_by_key(&config, "width"));
    }
    bool main_menu = true;
    SMALL_RECT windowSize = {0, 0, 15 * 2, 9}; // change the values
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
    while (main_menu)
    {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
        printf("================================\n"
               "          \e[41mSnake\e[43m \e[44mGame\e[0m\n"
               "\n");

        char choice[3][10] = {"Play", "Setting", "Exit"};
        for (unsigned i = 0; i < sizeof(choice) / sizeof(choice[0]); i++)
        {
            if (i == choiceSe)
            {
                printf("           \e[42;30m%d. %-s\033[0m", i + 1, choice[i]);
            }
            else
            {
                printf("           %d. %s", i + 1, choice[i]);
            }
            printf("\n");
        }
        printf("\n"
               " By %s\n Version \e[0;91midk!\e[0m",
               maker);
        printf("\n================================");
        setvbuf(stdout, buf, _IOFBF, sizeof(buf));
        char input = _getch();
        // printf("%d", input);
        if ((input == 80 || input == 115) && choiceSe < (sizeof(choice) / sizeof(choice[0]) - 1))
        {
            choiceSe++;
        }
        else if ((input == 72 || input == 119) && choiceSe > 0)
        {
            choiceSe--;
        }
        else if (input == 13)
        {
            switch (choiceSe)
            {
            case 0: // Play
                main_menu = false;
                break;
            case 1: // Setting
                setting_page();
                system("cls");
                choiceSe = 0;
                SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
                break;
            case 2: // Exit
                exit(0);
                break;
            }
        } else if (input == 27) {
            exit(0);
        }
    }

    game_setup();
    lp = true;
    CreateThread(NULL, 0, game_keyboard, NULL, 0, NULL);
    system("cls");
    while (lp)
    {
        reset_pixels();
        game_loop();
        game_draw();
        Sleep(delay);
        updateCount++;
    }

    return 0;
}

void setting_page()
{
    system("cls");
    choiceSe = 0;
    SMALL_RECT windowSize = {0, 0, 15 * 2, 13}; // change the values
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
    bool setting_menu = true;
    while (setting_menu)
    {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
        printf("================================\n"
               "       \e[41mSnake\e[43m \e[44mGame\e[43m \e[42mSetting\e[0m\n"
               "\n");
        char choice[7][15] = {"2 Player", "Foods", "FPS", "Height", "Width", "Reset M Tail", "Back"};
        switch (choiceSe)
        {
        case 1: // Foods
            choiceValInt = &foods;
            break;
        case 2: // FPS
            choiceValInt = &fps;
            break;
        case 3: // Height
            choiceValInt = &height;
            break;
        case 4: // Width
            choiceValInt = &width;
            break;
        }
        for (unsigned i = 0; i < sizeof(choice) / sizeof(choice[0]); i++)
        {
            if (i == choiceSe)
            {
                printf("         \e[42;30m%d. %-s\033[0m", i + 1, choice[i]);
                switch (i)
                {
                case 0:
                    if (player2)
                    {
                        printf(" \e[43;30mON\e[0m     ");
                    }
                    else
                    {
                        printf(" \e[43;30mOFF\e[0m     ");
                    }
                    break;
                case 5:
                case 6:
                    break;
                default:
                    if (choiceValSe)
                    {
                        printf(" \e[43;30m%d\e[0m     ", *choiceValInt);
                    }
                    else
                    {
                        printf(" %d", *choiceValInt);
                    }
                    break;
                }
            }
            else
            {
                printf("         %d. %-s", i + 1, choice[i]);
                switch (i)
                {
                case 0:
                    if (player2)
                    {
                        printf(" ON     ");
                    }
                    else
                    {
                        printf(" OFF     ");
                    }
                    break;
                case 1:
                    printf(" %d     ", foods);
                    break;
                case 2:
                    printf(" %d     ", fps);
                    break;
                case 3:
                    printf(" %d     ", height);
                    break;
                case 4:
                    printf(" %d     ", width);
                    break;
                case 5:
                case 6:
                    break;
                }
            }
            printf("\n");
        }
        printf("\n"
               " By %s\n Version \e[0;91midk!\e[0m",
               maker);
        printf("\n================================");
        setvbuf(stdout, buf, _IOFBF, sizeof(buf));
        char input = _getch();
        if ((input == 80 || input == 115) && choiceSe < (sizeof(choice) / sizeof(choice[0]) - 1))
        {
            if (choiceValSe)
            {
                if (*choiceValInt > 1 && !(choiceSe == 3 && *choiceValInt < 11) && !(choiceSe == 4 && *choiceValInt < 11))
                {
                    *choiceValInt -= 1;
                }
            }
            else
            {
                choiceSe++;
            }
        }
        else if ((input == 72 || input == 119) && choiceSe > 0)
        {
            if (choiceValSe)
            {
                if (!(choiceSe == 1 && *choiceValInt > 9) && !(choiceSe == 2 && *choiceValInt > 239) && !(choiceSe == 3 && *choiceValInt > 55) && !(choiceSe == 4 && *choiceValInt > 58))
                {
                    *choiceValInt += 1;
                }
            }
            else
            {
                choiceSe--;
            }
        }
        else if (input == 13)
        {
            switch (choiceSe)
            {
            case 0: // 2 Player
                player2 = !player2;
                break;
            case 5:
                max_tail = 0;
                printf("\e[92mReset Maximum Tail\e[0m");
                break;
            case 6: // Back
                setting_menu = false;
                break;
            default:
                choiceValSe = !choiceValSe;
                break;
            }
        } else if (input == 27) {
            setting_menu = false;
            break;
        }
    }
    set_config_value_by_key(&config, "2player", stringify_bool(player2));
    set_config_value_by_key(&config, "foods", stringify_int(foods));
    set_config_value_by_key(&config, "max_tail", stringify_int(max_tail));
    set_config_value_by_key(&config, "fps", stringify_int(fps));
    set_config_value_by_key(&config, "height", stringify_int(height));
    set_config_value_by_key(&config, "width", stringify_int(width));
    write_config(filename, &config);
}

void game_setup()
{
    setvbuf(stdout, buf, _IOFBF, sizeof(buf));

    SMALL_RECT windowSize = {0, 0, (short)((width * 2) + 1), ((short)height + 6)}; // change the values
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);
    snk1 = snake_create();
    snk1->player = 1;
    if(player2) {
        snk2 = snake_create();
        snk2->player = 2;
        snk1->pos.x = (width / 4) * 1;
        snk1->pos.y = height / 2;
        snk2->pos.x = (width / 4) * 3;
        snk2->pos.y = height / 2;
    } else {
        snk1->pos.x = width / 2;
        snk1->pos.y = height / 2;
    }
    for (unsigned int i = 0; i < foods; i++)
    {
        food[i] = food_create();
    }
    pix = (unsigned int **)malloc(height * sizeof(unsigned int *));
    for (unsigned i = 0; i < height; i++)
    {
        pix[i] = (unsigned int *)malloc(width * sizeof(unsigned int));
    }
    frameRate(fps);
}

DWORD WINAPI game_keyboard(void *data)
{
    while (lp)
    {
        if (player2) {
            if (GetKeyState(VK_LEFT) & 0x8000)
            {
                snake_set_move(snk2, -1, 0);
            }
            else if (GetKeyState(VK_RIGHT) & 0x8000)
            {
                snake_set_move(snk2, 1, 0);
            }
            else if (GetKeyState(VK_UP) & 0x8000)
            {
                snake_set_move(snk2, 0, -1);
            }
            else if (GetKeyState(VK_DOWN) & 0x8000)
            {
                snake_set_move(snk2, 0, 1);
            }
        }
        if (GetKeyState('A') & 0x8000 || (GetKeyState(VK_LEFT) & 0x8000 && !player2))
        {
            snake_set_move(snk1, -1, 0);
        }
        else if (GetKeyState('D') & 0x8000 || (GetKeyState(VK_RIGHT) & 0x8000 && !player2))
        {
            snake_set_move(snk1, 1, 0);
        }
        else if (GetKeyState('W') & 0x8000 || (GetKeyState(VK_UP) & 0x8000&& !player2))
        {
            snake_set_move(snk1, 0, -1);
        }
        else if (GetKeyState('S') & 0x8000 || (GetKeyState(VK_DOWN) & 0x8000&& !player2))
        {
            snake_set_move(snk1, 0, 1);
        }
    }
}

void game_loop()
{
    food_draw(food);
    snake_update(snk1);
    if(player2) {
        snake_update(snk2);
    }
    snake_check_die(snk1, snk2);
    snake_check_food(snk1, food);
    if(player2) {
        snake_check_food(snk2, food);
    }
    snake_draw(snk1);
    if(player2) {
        snake_draw(snk2);
    }
}

void game_draw()
{
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
    char text[] = "\e[41mSnake\e[43m \e[44mGame\e[0m";
    printf("%*s               \n", (width + ((sizeof(text) / sizeof(text[0])) / 2)) + 11, text);
    if(player2) {
        printf("Player1 Tail: %d     \n", snk1->length);
        printf("Player2 Tail: %d      \n", snk2->length);
    } else {
        printf("Tail: %d     \n", snk1->length);
        printf("Maximum Tail: %d      \n", max_tail);
    }
    for (unsigned i = 0; i < height + 2; i++)
    {
        printf("|");
        for (unsigned j = 0; j < width; j++)
        {
            if (i == 0 || i >= height + 1)
            {
                printf("==");
            }
            else if (pix[i - 1][j] == 1)
            {
                printf("\e[91m[1\e[0m");
            }
            else if (pix[i - 1][j] == 2)
            {
                printf("\e[31m(1\e[0m");
            }
            else if (pix[i - 1][j] == 11)
            {
                printf("\e[34m[2\e[0m");
            }
            else if (pix[i - 1][j] == 12)
            {
                printf("\e[94m(2\e[0m");
            }
            else if (pix[i - 1][j] == 50)
            {
                printf("\e[33m@:\e[0m");
            }
            else
            {
                printf("  ");
            }
        }
        printf("|\n");
    }
    setvbuf(stdout, buf, _IOFBF, sizeof(buf));
}

void frameRate(unsigned x)
{
    delay = 1000 / x;
}

void reset_pixels()
{
    for (unsigned i = 0; i < height; i++)
    {
        for (unsigned j = 0; j < width; j++)
        {
            pix[i][j] = 0;
        }
    }
}

Snake *snake_create()
{
    Snake *snk = (Snake *)malloc(sizeof(Snake));
    snk->speed.x = 0;
    snk->speed.y = 0;
    snk->input.x = 0;
    snk->input.y = 0;
    snk->length = 1;
    snk->tail = (Vector *)malloc(width * height * sizeof(Vector));
    return snk;
}

void snake_update(Snake *snk)
{

    if (snk->speed.x != 1 && snk->input.x == -1)
    {
        snk->speed.x = -1;
        snk->speed.y = 0;
    }
    else if (snk->speed.x != -1 && snk->input.x == 1)
    {
        snk->speed.x = 1;
        snk->speed.y = 0;
    }
    else if (snk->speed.y != 1 && snk->input.y == -1)
    {
        snk->speed.x = 0;
        snk->speed.y = -1;
    }
    else if (snk->speed.y != -1 && snk->input.y == 1)
    {
        snk->speed.x = 0;
        snk->speed.y = 1;
    }

    snk->pos.x += snk->speed.x;
    snk->pos.y += snk->speed.y;

    snk->tail[snk->length].x = snk->pos.x;
    snk->tail[snk->length].y = snk->pos.y;

    for (unsigned i = 0; i < snk->length; i++)
    {
        snk->tail[i].x = snk->tail[i + 1].x;
        snk->tail[i].y = snk->tail[i + 1].y;
    }
}

void snake_draw(Snake *snk)
{
    unsigned int a = 0;
    if(snk->player == 2) {
        a = 10;
    }
    for (unsigned i = 0; i < snk->length; i++)
    {
        pix[snk->tail[i].y][snk->tail[i].x] = a + 1;
    }
    pix[snk->pos.y][snk->pos.x] = a + 2;
}

void snake_set_move(Snake *snk, int x, int y)
{
    snk->input.x = x;
    snk->input.y = y;
}

void snake_die(Snake *snk)
{
    // setvbuf(stdout, NULL, _IOFBF, 1);
    char text[20] = "";
    if(player2) {
        if(snk->player == 1) {
            strcpy(text, "Player 1 Lost!");
        } else {
            strcpy(text, "Player 2 Lost!");
        }
    } else {
        strcpy(text, "GAME OVER");
    }
    printf("\e[0;31m%*s\e[0m\n", width + ((sizeof(text) / sizeof(text[0])) / 2), text);
    setvbuf(stdout, buf, _IOFBF, sizeof(buf));
    system("pause");
    // SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
    max_tail = max(snk1->length, max_tail);
    set_config_value_by_key(&config, "max_tail", stringify_int(max_tail));
    write_config(filename, &config);
    free(pix);
    // set to config
    for (unsigned int i = 0; i < foods; i++)
    {
        food_destroy(food[i]);
    }
    if(player2) {
        snake_destroy(snk2);
    }
    snake_destroy(snk1);
    lp = false;
    system("cls");
    main();
}

void snake_check_die(Snake *snk1, Snake *snk2)
{
    if (snk1->pos.x < 0 || snk1->pos.x >= width || snk1->pos.y < 0 || snk1->pos.y >= height)
    {
        snake_die(snk1);
    } 

    // make hit tall died
    for (unsigned i = snk1->length; i > 1; i--)
    {
        if (snk1->pos.x == snk1->tail[i - 2].x && snk1->pos.y == snk1->tail[i - 2].y)
        {
            snake_die(snk1);
            break;
        }  
        if(player2) {
            if (snk2->pos.x == snk1->tail[i - 2].x && snk2->pos.y == snk1->tail[i - 2].y)
            {
                snake_die(snk2);
                break;
            }
        }
    }
    
    if(player2) {
        if(snk2->pos.x < 0 || snk2->pos.x >= width || snk2->pos.y < 0 || snk2->pos.y >= height) {
            snake_die(snk2);
        }

        for (unsigned i = snk2->length; i > 1; i--)
        {
            if (snk2->pos.x == snk2->tail[i - 2].x && snk2->pos.y == snk2->tail[i - 2].y)
            {
                snake_die(snk2);
                break;
            } else if (snk1->pos.x == snk2->tail[i - 2].x && snk1->pos.y == snk2->tail[i - 2].y)
            {
                snake_die(snk1);
                break;
            }
        }
    }
    

    


}

void snake_check_food(Snake *snk, Food *fd[10])
{
    for (unsigned i = 0; i < foods; i++)
    {
        if (snk->pos.x == fd[i]->pos.x && snk->pos.y == fd[i]->pos.y)
        {
            // food_new_food(fd);
            fd[i]->pos.x = rand() % width;
            fd[i]->pos.y = rand() % height;
            snk->length++;
        }
    }
}

void snake_destroy(Snake *snk)
{
    free(snk->tail);
    free(snk);
}

Food *food_create()
{
    Food *fd = (Food *)malloc(sizeof(Food));
    fd->pos.x = rand() % width;
    fd->pos.y = rand() % height;
    return fd;
}

void food_draw(Food *fd[10])
{
    for (unsigned int i = 0; i < foods; i++)
    {
        pix[fd[i]->pos.y][fd[i]->pos.x] = 50;
    }
}

void food_destroy(Food *fd)
{
    free(fd);
}

int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && tolower((unsigned char)*s1) == tolower((unsigned char)*s2))
    {
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

char *stringify_int(int value)
{
    char str[MAX_VALUE_LEN];
    snprintf(str, MAX_VALUE_LEN, "%d", value);
    return str;
}

char *stringify_float(float value)
{
    char str[MAX_VALUE_LEN];
    snprintf(str, MAX_VALUE_LEN, "%f", value);
    return str;
}

char *stringify_bool(bool value)
{
    char str[MAX_VALUE_LEN];
    snprintf(str, MAX_VALUE_LEN, "%s", value ? "true" : "false");
    return str;
}

int parse_int(const char *str)
{
    int *value = (int *)malloc(sizeof(int));
    sscanf(str, "%d", value);
    return *value;
}

float parse_float(const char *str)
{
    float *value = (float *)malloc(sizeof(float));
    sscanf(str, "%f", value);
    return *value;
}

bool parse_bool(const char *str)
{
    bool *value = (bool *)malloc(sizeof(bool));
    if (strcasecmp(str, "true") == 0)
    {
        *value = true;
    }
    else if (strcasecmp(str, "false") == 0)
    {
        *value = false;
    }
    else
    {
        fprintf(stderr, "Invalid boolean value: %s\n", str);
        free(value);
        return NULL;
    }
    return *value;
}

void write_config(const char *filename, ConfigFile *config)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }
    for (int i = 0; i < config->count; ++i)
    {
        fprintf(file, "%s=%s\n", config->entries[i].key, config->entries[i].value);
    }
    fclose(file);
}

void read_config(const char *filename, ConfigFile *config)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file for reading");
        return;
    }
    config->count = 0;
    while (config->count < MAX_ENTRIES && fscanf(file, "%255[^=]=%255[^\n]\n", config->entries[config->count].key, config->entries[config->count].value) == 2)
    {
        config->count++;
    }
    fclose(file);
}

const char *get_config_value_by_key(const ConfigFile *config, const char *key)
{
    for (int i = 0; i < config->count; ++i)
    {
        if (strcasecmp(config->entries[i].key, key) == 0)
        {
            return config->entries[i].value;
        }
    }
    return NULL;
}

void set_config_value_by_key(ConfigFile *config, const char *key, const char *value)
{
    for (int i = 0; i < config->count; ++i)
    {
        if (strcasecmp(config->entries[i].key, key) == 0)
        {
            strncpy(config->entries[i].value, value, MAX_VALUE_LEN - 1);
            config->entries[i].value[MAX_VALUE_LEN - 1] = '\0';
            return;
        }
    }
    if (config->count < MAX_ENTRIES)
    {
        strncpy(config->entries[config->count].key, key, MAX_KEY_LEN - 1);
        config->entries[config->count].key[MAX_KEY_LEN - 1] = '\0';
        strncpy(config->entries[config->count].value, value, MAX_VALUE_LEN - 1);
        config->entries[config->count].value[MAX_VALUE_LEN - 1] = '\0';
        config->count++;
    }
}

void delete_config_entry_by_key(ConfigFile *config, const char *key)
{
    for (int i = 0; i < config->count; ++i)
    {
        if (strcasecmp(config->entries[i].key, key) == 0)
        {
            for (int j = i; j < config->count - 1; ++j)
            {
                config->entries[j] = config->entries[j + 1];
            }
            config->count--;
            return;
        }
    }
}