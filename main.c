#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define COIN_KINDS   5U
#define TOY_COUNT    4U
#define NAME_LEN    16U
#define BAR_WIDTH   20U

/* the coins we know about, in piastres */
static const uint16_t COIN_VALUE[COIN_KINDS] = { 25U, 50U, 100U, 200U, 500U };

/* how many of each coin are in the bank right now */
static uint16_t coinCount[COIN_KINDS];

typedef struct {
    char     name[NAME_LEN];
    uint32_t price;        /* in piastres */
} Toy_t;

static Toy_t shop[TOY_COUNT];

/* ---- helpers ---- */
static void     clearInputLine(void);
static int      readMenuChoice(void);
static int      readPositiveInt(const char *prompt);

/* ---- required functions ---- */
static void     seedBank(void);
static void     addCoins(void);
static void     takeCoins(void);
static uint32_t bankTotal(void);
static uint32_t sumCoins(const uint16_t *counts, uint8_t n);
static uint8_t  biggestPile(void);
static void     showBank(void);
static void     drawBar(uint16_t value, uint16_t full, uint8_t width);
static void     buyToy(void);
static void     bankReport(void);

/* ================= helpers ================= */

/* Throw away everything left on the current input line, up to and
   including the newline (or EOF). Used after a bad scanf so the menu
   never loops forever on leftover junk like "hello". */
static void clearInputLine(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

/* Reads one menu number. Returns the number typed, or -1 if the
   person typed something that is not a whole number at all. */
static int readMenuChoice(void)
{
    int choice;
    int ok;

    printf("> ");
    ok = scanf("%d", &choice);
    clearInputLine();

    if (ok != 1) {
        return -1;
    }
    return choice;
}

/* Asks for a non-negative whole number with the given prompt.
   Returns the number, or -1 if the input was not a valid
   non-negative number. */
static int readPositiveInt(const char *prompt)
{
    int value;
    int ok;

    printf("%s", prompt);
    ok = scanf("%d", &value);
    clearInputLine();

    if (ok != 1 || value < 0) {
        return -1;
    }
    return value;
}

/* ================= required functions ================= */

static void seedBank(void)
{
    static const uint16_t startCoins[COIN_KINDS] = { 4U, 3U, 2U, 1U, 0U };
    static const char     names[TOY_COUNT][NAME_LEN] =
        { "Ball", "Kite", "Robot", "Bike" };
    static const uint32_t prices[TOY_COUNT] = { 150U, 300U, 900U, 2500U };
    uint8_t i;

    for (i = 0U; i < COIN_KINDS; i++) {
        coinCount[i] = startCoins[i];
    }
    for (i = 0U; i < TOY_COUNT; i++) {
        strncpy(shop[i].name, names[i], NAME_LEN - 1U);
        shop[i].name[NAME_LEN - 1U] = '\0';
        shop[i].price = prices[i];
    }
    printf("A fresh piggy bank is ready!\n");
}

static void addCoins(void)
{
    int coinIdx;
    int qty;

    coinIdx = readPositiveInt("Which coin (0=25,1=50,2=100,3=200,4=500)? ");
    if (coinIdx < 0 || (uint32_t)coinIdx >= COIN_KINDS) {
        printf("That coin does not exist. Nothing changed.\n");
        return;
    }

    qty = readPositiveInt("How many? ");
    if (qty < 0) {
        printf("That is not a valid amount. Nothing changed.\n");
        return;
    }

    coinCount[coinIdx] = (uint16_t)(coinCount[coinIdx] + (uint16_t)qty);
    printf("Added %d coin(s).\n", qty);
}

static void takeCoins(void)
{
    int coinIdx;
    int qty;

    coinIdx = readPositiveInt("Which coin (0=25,1=50,2=100,3=200,4=500)? ");
    if (coinIdx < 0 || (uint32_t)coinIdx >= COIN_KINDS) {
        printf("That coin does not exist. Nothing changed.\n");
        return;
    }

    qty = readPositiveInt("How many? ");
    if (qty < 0) {
        printf("That is not a valid amount. Nothing changed.\n");
        return;
    }

    if ((uint16_t)qty > coinCount[coinIdx]) {
        printf("The bank does not have that many. Nothing changed.\n");
        return;
    }

    coinCount[coinIdx] = (uint16_t)(coinCount[coinIdx] - (uint16_t)qty);
    printf("Took out %d coin(s).\n", qty);
}

/* Sums coinCount[i] * COIN_VALUE[i] for every coin kind. */
static uint32_t bankTotal(void)
{
    uint16_t weighted[COIN_KINDS];
    uint8_t  i;

    for (i = 0U; i < COIN_KINDS; i++) {
        weighted[i] = (uint16_t)(coinCount[i] * COIN_VALUE[i]);
    }
    return sumCoins(weighted, COIN_KINDS);
}

/* Recursive sum of an array. No loops allowed in here. */
static uint32_t sumCoins(const uint16_t *counts, uint8_t n)
{
    if (n == 0U) {
        return 0U;
    }
    return (uint32_t)counts[n - 1U] + sumCoins(counts, (uint8_t)(n - 1U));
}

/* Index of the coin kind we own the most of. Ties go to the
   smaller index. */
static uint8_t biggestPile(void)
{
    uint8_t best = 0U;
    uint8_t i;

    for (i = 1U; i < COIN_KINDS; i++) {
        if (coinCount[i] > coinCount[best]) {
            best = i;
        }
    }
    return best;
}

static void drawBar(uint16_t value, uint16_t full, uint8_t width)
{
    uint8_t filled;
    uint8_t i;

    if (full == 0U) {
        filled = (value > 0U) ? width : 0U;
    } else {
        uint32_t scaled = (uint32_t)value * width / full;
        filled = (scaled > width) ? width : (uint8_t)scaled;
    }

    for (i = 0U; i < width; i++) {
        putchar((i < filled) ? '#' : '-');
    }
}

static void showBank(void)
{
    uint16_t tallest = coinCount[biggestPile()];
    uint8_t  i;

    printf("\n-- Piggy Bank --\n");
    for (i = 0U; i < COIN_KINDS; i++) {
        printf("%4u pt x %3u  ", COIN_VALUE[i], coinCount[i]);
        drawBar(coinCount[i], (tallest == 0U) ? 1U : tallest, BAR_WIDTH);
        printf("\n");
    }
}

static void buyToy(void)
{
    uint32_t total = bankTotal();
    int      pick;
    uint8_t  i;

    printf("\n-- Toy Shop --\n");
    for (i = 0U; i < TOY_COUNT; i++) {
        printf("%u) %-8s %5u pt\n", i, shop[i].name, shop[i].price);
    }

    pick = readPositiveInt("Which toy? ");
    if (pick < 0 || (uint32_t)pick >= TOY_COUNT) {
        printf("That toy does not exist.\n");
        return;
    }

    if (total >= shop[pick].price) {
        printf("You can buy the %s! You'd have %u pt left.\n",
               shop[pick].name, total - shop[pick].price);
    } else {
        printf("Not enough. You need %u more pt for the %s.\n",
               shop[pick].price - total, shop[pick].name);
    }
}

static void bankReport(void)
{
    uint32_t total = bankTotal();
    uint8_t  top   = biggestPile();
    uint32_t coinsAltogether = sumCoins(coinCount, COIN_KINDS);
    uint8_t  affordable = 0U;
    uint8_t  i;

    for (i = 0U; i < TOY_COUNT; i++) {
        if (total >= shop[i].price) {
            affordable++;
        }
    }

    printf("\n-- Bank Report --\n");
    printf("Total money      : %u pt\n", total);
    printf("Coins altogether : %u\n", coinsAltogether);
    printf("Biggest pile     : %u pt coins (%u of them)\n",
           COIN_VALUE[top], coinCount[top]);
    printf("Toys affordable  : %u out of %u\n", affordable, TOY_COUNT);
}

/* ================= menu ================= */

int main(void)
{
    int choice;

    seedBank();

    do {
        printf("\n===== Piggy Bank Menu =====\n");
        printf("1) Add coins\n");
        printf("2) Take out coins\n");
        printf("3) Show bank\n");
        printf("4) Buy a toy\n");
        printf("5) Day report\n");
        printf("0) Quit\n");

        choice = readMenuChoice();

        switch (choice) {
            case 1: addCoins();   break;
            case 2: takeCoins();  break;
            case 3: showBank();   break;
            case 4: buyToy();     break;
            case 5: bankReport(); break;
            case 0: printf("Bye!\n"); break;
            default:
                printf("Please type a number from the menu.\n");
                break;
        }
    } while (choice != 0);

    return 0;
}