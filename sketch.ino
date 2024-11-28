#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>


#define OLED_RESET -1
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define CLK 7
#define DIO 6
TM1637Display sevenSegment(CLK, DIO);

const int brickStartX = 5; 
const int brickStartY = 5; 
const int brickWidth = 16; 
const int brickHeight = 5; 
const int brickGap = 2; 


bool bricks[5][12];

// pinler
const int ledPins[] = {2, 3, 4}; 
const int buttonSelect = 11; 
const int buttonUp = 12; 
const int buttonDown = 13;
int selectedOption = 0; 

const int potPin = A0;
const int LDR_PIN = 8;


int level = 1; 
int score = 0;
int lives = 3; 
float ballSpeed = 1.0;
bool gameRunning = false; 
int paddleX, paddleY; // palet pozisyonu
int ballX, ballY; // top pozisyonu
int ballDX = 1; 
int ballDY = -1; 
const int paddleWidth = 30;
const int paddleHeight = 2;
const int ballSize = 4; // top boyutu

bool brickDesigns[5][7][7] = {
    {
        {0, 1, 1, 0, 1, 1, 0},
        {1, 0, 1, 1, 1, 0, 1},
        {1, 1, 0, 1, 0, 1, 1}
    },
    {
        {1, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0, 1, 0}
    },
    {
        {0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 0}
    },
    {
        {1, 1, 0, 1, 0, 1, 1},
        {0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0}
    },
    {
        {1, 0, 1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1, 0, 1},
        {0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 0, 1, 0, 1}
    }
};

const int brickRows[] = {3, 4, 4, 5, 5}; //tuğla satır sayısı
const int brickCols[] = {7, 7, 7, 7, 7}; //tuğla sütun sayısı


struct PowerUp {
    int x;
    int y;
    bool active; // obeje mevcut mu kontrolü
};

PowerUp powerUps[5]; // max 5 tuğla olabilir

// initializeBricks fonksiyonunda tuğlaları başlatma
void initializeBricks() {
    
    for (int i = 0; i < brickRows[level - 1]; i++) {
        for (int j = 0; j < brickCols[level - 1]; j++) {
            bricks[i][j] =      brickDesigns[level - 1][i][j]; 
        }
    }
}

void setup() {
    Serial.begin(9600);
    
    // oled ekran ayarları

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();
    
    // 7 segment ekran ayarları

    sevenSegment.setBrightness(0x0F);
    sevenSegment.showNumberDec(score, false);
    
    // led pinlerin ayarlanamsı
    for (int i = 0; i < 3; i++) {

        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], HIGH); // can göstermek için ledler
    }
    
    // buton pinlerini ayarlama
    pinMode(buttonSelect, INPUT_PULLUP);
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    
    // palet ve top pozisyonları
    paddleX = (SCREEN_WIDTH - paddleWidth) / 2;
    paddleY = SCREEN_HEIGHT - paddleHeight - 2; // palet alt kenardan 2 br yüksekte
    ballX = SCREEN_WIDTH / 2 - ballSize / 2;
    ballY = SCREEN_HEIGHT / 2 - ballSize / 2;
    
    initializeBricks();
    
    showMenu();
}

void loop() {
    // menü kontrolü
    if (!gameRunning) {
        navigateMenu();
    } else {
        updateGame();
        updatePowerUps(); // düşen objeler temizlenir
    }
}

void showMenu() {
    // menü tekrarlanır
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    
    if (selectedOption == 0) {

        // eğer start seçildiyse
        display.setCursor(20, 20);
        display.setTextColor(BLACK, WHITE);
        display.println("Baslat");
        display.setTextColor(WHITE);
        display.setCursor(20, 40);
        display.println("Cikis");

    } 
    else if (selectedOption == 1) {
        // eğer exit seçildiyse
        
        display.setCursor(20, 20);
        
        display.println("Baslat");
        //display.setTextColor(WHITE);
        display.setCursor(20, 40);
        display.setTextColor(BLACK, WHITE); 
        display.println("Cikis");
        
        }
    
    display.display();
}

void navigateMenu() {
    //int selectedOption = 0;
     // 0-> start 
    //1 -> exit
    
    // menüde gezinme
    while (!gameRunning) {
        // yukarı düğmesi kontrolü
        if (digitalRead(buttonUp) == LOW) {
            selectedOption =0;
            selectedOption = (selectedOption + 2) % 2; // seçimi döngüsel olarak yukarı taşı
            showMenu(); // ekranı güncelle
            delay(200); 
        }

        if (digitalRead(buttonDown) == LOW) {

            selectedOption =0;
            selectedOption = (selectedOption + 1) % 2; // seçimi döngüsel olarak aşağı taşı
            showMenu(); // ekranı güncelle
            delay(200); 
        }
        
        if (digitalRead(buttonSelect) == LOW) {
            if (selectedOption == 0) {
                // start seçildi
                gameRunning = true;
                display.clearDisplay();
                initializeGame(); // oyun başlatılır
            } else {
                // exit seçildi
                display.clearDisplay();
                display.setTextSize(2);
                display.setTextColor(WHITE);
                display.setCursor(0, 0);
                display.println("Oyuna ilgin");
                display.println("icin tesekkurler");
                display.display();
                delay(2000); // mesaj görülsün diye bekleme
                // menüden çıkış
                while (true) {
                    // Programı durdurmak için sonsuz döngü
                }
            }
            delay(200); 
        }
    }
}

// tuğlaların bitip bitmediğini kontrol etme
bool checkLevelComplete() {
    for (int i = 0; i < brickRows[level - 1]; i++) {
        for (int j = 0; j < brickCols[level - 1]; j++) {
            if (bricks[i][j]) {

                return false; // tuğla varsa seviye bitmedi
            }
        }
    }
    return true; // tuğlalar bitti seviye geçildi
}

void nextLevel() {
    if (level < 5) {
        // seviye sınırını kontrol etme
        level++; // seviyeyi artır
        initializeBricks(); // yeni seviye için tuğlalar
        resetBallAndPaddle(); // top ve paletin yeniden başlatılması
        
        // canı sıfırlama
        lives = 3;
        
        // topun hızına artır
        ballSpeed += 0.2;
    } else {
        // eğer max değer geldiyse oyunu bitir
        gameRunning = false;
        displayGameComplete(); // oyunun bitiş mesajı
        delay(3000);
        showMenu();
        
    }
    updateLives(); // can ledini güncelleme
}

void displayGameComplete() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 20);
    display.println("Kazandin");
    display.setCursor(20, 40);
    display.print(F("Skor: "));
    display.println(score);
    display.display();
}

    // oyun güncelleme kısmında tuğlaların   bitip bitmediğini kontrol etme
void updateGame() {
    // palet kontrolü
    int potValue = analogRead(potPin);
    paddleX = map(potValue, 0, 1023, 0, SCREEN_WIDTH - paddleWidth);
    
    // top hareketi
    ballX += ballDX * ballSpeed;
    ballY += ballDY * ballSpeed;
    
    // topun ekran kenarlarına çarpması
    if (ballX <= 0 || ballX >= SCREEN_WIDTH - ballSize) {
        ballDX = -ballDX;
    }
    if (ballY <= 0) {
        ballDY = -ballDY;
    }
    
    // palet çarpması
    if (ballY >= paddleY - ballSize && ballX >= paddleX && ballX <= paddleX + paddleWidth) {
        ballDY = -ballDY;
        // paletin çarpma noktasına göre    topun yönünü ayarla
        int paddleCenter = paddleX + paddleWidth / 2;
        int ballCenter = ballX + ballSize / 2;
        if (ballCenter < paddleCenter) {
            ballDX = -ballSpeed;
        } else if (ballCenter > paddleCenter) {
            ballDX = ballSpeed;
        } else {
            ballDX = 0;
        }
    }
    
    // topun aşağı düşmesi
    if (ballY >= SCREEN_HEIGHT) {
        lives--;
        updateLives();
        if (lives <= 0) {
            // can bittiğinde oyun sonu
            gameRunning = false;
            displayGameOver();
            delay(3000); // oyun sonu ekranı için bekleme
            
            showMenu();
        } else {
            // can kaldıysa topu ve paleti sıfırla
            resetBallAndPaddle();
        }
    }
    
    checkBrickCollision();
    
    // tuğlaların bitip bitmediğini kontrol et
    if (checkLevelComplete()) {
        nextLevel(); 
    }
    
    lightThemeControl();
    
    drawGame();
}

// canı güncelleme fonksiyonu
void updateLives() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(ledPins[i], i < lives ? HIGH : LOW);
    }
}

// oyun sonu ekranı
void displayGameOver() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 20);
    
    display.print(F("Skor:"));
    display.println(score);
    display.display();
}

// top ve paleti sıfırlama
void resetBallAndPaddle() {
    ballX = SCREEN_WIDTH / 2 - ballSize / 2;
    ballY = SCREEN_HEIGHT / 2 - ballSize / 2;
    ballDX = ballSpeed;
    ballDY = -ballSpeed;
    paddleX = (SCREEN_WIDTH - paddleWidth) / 2;
}

// tuğla çarpışması kontrolü
void checkBrickCollision() {
    for (int i = 0; i < brickRows[level - 1]; i++) {
        for (int j = 0; j < brickCols[level - 1]; j++) {
            if (bricks[i][j]) {
                int brickX = brickStartX + j * (brickWidth + brickGap);
                int brickY = brickStartY + i * (brickHeight + brickGap);
                
                // top ile tuğla çarpışması
                if (ballX + ballSize >= brickX && ballX <= brickX + brickWidth &&
                    ballY + ballSize >= brickY && ballY <= brickY + brickHeight) {
                    ballDY = -ballDY;
                    bricks[i][j] = false;
                    score++;
                    sevenSegment.showNumberDec(score, false);
                    
                    // %10 şansla obje düşürme
                    if (random(100) < 10) {
                        // ilk boş obje yeri
                        for (int k = 0; k < 5; k++) {
                            if (!powerUps[k].active) {
                                powerUps[k].x = brickX + brickWidth / 2;
                                powerUps[k].y = brickY;
                                powerUps[k].active = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}


void drawGame() {
    //oyunun genel çizimi
    display.clearDisplay();
    
    // tuğlaları çizme
    for (int i = 0; i < brickRows[level - 1]; i++) {
        for (int j = 0; j < brickCols[level - 1]; j++) {
            if (bricks[i][j]) {
                int brickX = brickStartX + j * (brickWidth + brickGap);
                int brickY = brickStartY + i * (brickHeight + brickGap);
                display.fillRect(brickX, brickY, brickWidth, brickHeight, WHITE);
            }
        }
    }
    
    // paleti çizme
    display.fillRect(paddleX, paddleY, paddleWidth, paddleHeight, WHITE);
    
    // topu çizme
    display.fillRect(ballX, ballY, ballSize, ballSize, WHITE);
    
    // düşen objeleri çizme
    for (int i = 0; i < 5; i++) {
        if (powerUps[i].active) {
            display.fillRect(powerUps[i].x, powerUps[i].y, 4, 4, WHITE);
        }
    }
    
    display.display();
}

void lightThemeControl() {
    if (digitalRead(LDR_PIN) == LOW) {
        // karanlık mod
        display.invertDisplay(false); // metin siyah, arka plan beyaz
    } else {
        // aydınlık mod
        display.invertDisplay(true);  // metin beyaz, arka plan siyAH
    }
}

// düşen objeleri güncelleme
void updatePowerUps() {
    for (int i = 0; i < 5; i++) {
        if (powerUps[i].active) {
            // objenin hareketi
            powerUps[i].y += 1;
            
            // obje ile palet etkileşim kontrolü
            if (powerUps[i].y >= paddleY && powerUps[i].x >= paddleX && powerUps[i].x <= paddleX + paddleWidth) {
                lives++; // can artar
                updateLives();
                powerUps[i].active = false; // obje devre dışı bırakılır
            } else if (powerUps[i].y >= SCREEN_HEIGHT) {
                // obje eğer ekranın alt kısmına gelirse devre dışı bırak
                powerUps[i].active = false;
            }
        }
    }
}

// oyun başlatma
void initializeGame() {
    // parametreletin (hız, can, skor) sıfırlanma kısmıı
    ballX = SCREEN_WIDTH / 2 - ballSize / 2;
    ballY = SCREEN_HEIGHT / 2 - ballSize / 2;
    ballDX = 1;
    ballDY = -1;
    paddleX = (SCREEN_WIDTH - paddleWidth) / 2;
    lives = 3; // canı sıfırlar
    score = 0; // skoru sıfırlar
    ballSpeed = 1.0; // topun hızı sıfırlanır
    level = 1; // en başa geri dönüş

    // yeniden başlat
    initializeBricks();

    // kalan can için led güncellenmesi
    updateLives();

    // skor ve seviyenin güncellenmesi
    sevenSegment.showNumberDec(score, false);
}