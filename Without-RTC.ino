#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <EncButton.h>
EncButton<EB_TICK, 2, 3, 4> enc;


int Relay_first = 9; //Первое реле. Потом подключаем в каждый выход по реле
int Len_Menu = 3 - 1; //Длинна меню меняем первую цыфру

int Max_Chanel = 4 + 1; //Максимум каналов
int Sost_Chanel[] = {0, 0, 0, 0}; //Состояния каналов 0 или 1 (вкл или выкл)

int Menu[] = {1,2,3};//Номера Option
int Times_Hour_Start[] = {0,10,13};//Время часы
int Times_Minute_Start[] = {1,41,47};//Время минуты
int Timer[] = {1, 1, 12};//Таймер
int Chanel[] = {0, 0, 2};//Номер канала привязанный к Option
int Days_watering[][7] = {{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}}; //дни полива

int pos = 0;//Позиция в настройках
int pos_first = 0;//Позиция на начальном экране 
int Pos_CH = 1;//Позиция на экране всех каналов
int var = 0;//Регулировка начала регулировки времени старта полива (увеличение или уменьшение) 0 или 1
int list = 0; //Листать в Option
int pos_day = 0; //Позиция при выборе дня полива

bool Vkl = false;
bool Print_time = true; //Выводить начальный экран или настройки Options
bool settings_time = false;

//Время
uint32_t time;
int DAY = 0;
int HOUR = 0;
int MIN = 0;
int SEC = 0;
char Days[][10] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

//таймер сна экрана
int timeout = 60; //Время работы экрана в секундах
int cnt_time = 0; //Счетчик timeout
bool T_O = true; //Включен экран или нет в начале работы



void setup() {
  lcd.init(); // инициализация
  lcd.backlight(); //Включить подстветку экрана

  pinMode(Chanel[pos]+Relay_first, OUTPUT);//Говорим что пин, к которому подключено реле выдает напряжение

  time = millis();
  
}

void loop() {
  //Время
  if (millis()-time > 1000){
    time = millis();
    SEC++;
    if (T_O) cnt_time++;
    else cnt_time=0;
  }
  if (SEC == 60){
    MIN++;
    SEC = 0;
    lcd.clear();
  }
  if (MIN == 60){
    HOUR++;
    MIN = 0;
    lcd.clear();
  }
  if (HOUR == 24){
    DAY++;
    HOUR = 0;
    lcd.clear();
  }
  if (DAY == 7){
    DAY = 0;
    lcd.clear();
  }

  //Настройка таймаута
  if (cnt_time==timeout){
    T_O = false;
    lcd.noBacklight();
  }
  if (enc.click()) {
    lcd.backlight();
    T_O = true;
    cnt_time=0;
  }
  if (enc.turn()) {
    lcd.backlight();
    T_O = true;
    cnt_time=0;
  }


  //Для использования инкодера и кнопки
  enc.tick();

  //Начальный экран, если начать 4 раза, то можно перейти в настройки автополива по времени
  if (Print_time and not(settings_time)) {

    //Начальный экран времени
    if ((pos_first%2) == 0){
      lcd.setCursor(0, 0);
      lcd.print(HOUR);
      lcd.print(":");
      lcd.print(MIN);
      lcd.print(":");
      lcd.print(SEC);
      lcd.setCursor(0, 1);
      lcd.print(Days[DAY]);  
    }

    //Обработчик кручения и переход между страницами
    if (enc.right() and pos_first < Max_Chanel) {
      pos_first += 1;
      lcd.clear();
      }
    else if (enc.left() and pos_first > 0) {
      pos_first -= 1;
      lcd.clear();
      } 
    
    //Экран с каналами. Можно руками включить определенный канал
    if (pos_first%2 == 1){
      //Выводим каналы
      lcd.setCursor(0, 0);  
      lcd.print("Ch0 Ch1 Ch2 Ch3");
      lcd.setCursor(0, 1);  

      //Перемещение курсора между каналами на странице
      for (int j=0; j<Max_Chanel-1; j++){
        if (Sost_Chanel[j] == 1 and j==Pos_CH-1){
          lcd.print("ON< ");
        }
        else if (Sost_Chanel[j] == 0 and j==Pos_CH-1){
          lcd.print("OFF<");
        }
        else if (Sost_Chanel[j] == 1){
          lcd.print("ON  ");
        }
        else if (Sost_Chanel[j] == 0){
          lcd.print("OFF ");
        }
      }

      //Обработчик перемещения курсора
      if (enc.rightH()) {
        Pos_CH += 1;
      }
      else if (enc.leftH()) {
        Pos_CH-=1;
      } 

      //Если один клик, то значение канала меняется. То есть включается или выключается
      if (enc.click()) {
        if (Sost_Chanel[Pos_CH-1] == 1){
          Sost_Chanel[Pos_CH-1] = 0;
        }
        else if (Sost_Chanel[Pos_CH-1] == 0){
          Sost_Chanel[Pos_CH-1] = 1;
        }
        
      }
    }
  }

  //Настройки времени
  else if (Print_time and settings_time){
    lcd.setCursor(0, 0);
    lcd.print("Now time: ");
    lcd.print(HOUR);
    lcd.print(":");
    lcd.print(MIN);

    lcd.setCursor(0, 1);
    lcd.print(Days[DAY]);

    if (enc.right()) MIN++;
    else if (enc.left()) MIN--;

    if (enc.rightH()) {
      DAY++;
      lcd.clear();
    }
    else if (enc.leftH()){
      DAY--;
      lcd.clear();
    }
  }

  //Если нажать 4 раза, то появится меню настройки таймеров включения полива
  else{
    if (list == 0){
      //pos сразу же выбирает какой полив смотрим. Потом про помощи 2 и 3 перемещаемся

      //Вывод названия полива или номер 
      lcd.setCursor(0, 0);  
      lcd.print("Option");
      lcd.print(Menu[pos]);

      //Длительность полива в минутах
      lcd.print(" Timer ");
      lcd.print(Timer[pos]);

      //Вывод времени начала. Это все сделано для красивого вывода времени
      if (Times_Minute_Start[pos] < 10){
        lcd.setCursor(0, 1);
        lcd.print(Times_Hour_Start[pos]);
        lcd.print(":0");
        lcd.print(Times_Minute_Start[pos]);
      }
      else {
        lcd.setCursor(0, 1);
        lcd.print(Times_Hour_Start[pos]);
        lcd.print(":");
        lcd.print(Times_Minute_Start[pos]);
      }

      //Вывод канала, на котором должен включится полив
      lcd.print("  Ch");
      lcd.print(Chanel[pos]);
      lcd.print("  ");

      //Обработчик выключиния и включения канала в меню. Сразу же вывод на экран 
      if (Sost_Chanel[Chanel[pos]] == 1) {
        lcd.print(" ON");
      }
      else {
        lcd.print("OFF");
      }

      //Если 3 клика, то включаем или выключаем канал из меню
      if (enc.hasClicks(3)) {
        if (Sost_Chanel[Chanel[pos]] == 1){
          Sost_Chanel[Chanel[pos]] = 0;
        }
        else{
          Sost_Chanel[Chanel[pos]] = 1;
        }
      }
      
      //Обработчик поворота при нажатии. Регулировка таймера полива
      if (enc.rightH()) {
        Timer[pos] += 1;
        }
      else if (enc.leftH()) {
        Timer[pos] -= 1;
        if (Timer[pos]==9) lcd.clear();
        } 
      
      //Обработчики долгого нажатия. Уменьшает или увеличивает время начала полива. var рагулирует увеличивается время или уменьшается
      if (enc.hold() and var%2==0) {
        delay(100); //Можно менять. Влияет на скорость изменения времени.
        Times_Minute_Start[pos] += 1;
        if (Times_Minute_Start[pos]==60){
          Times_Minute_Start[pos]=0;
          Times_Hour_Start[pos] += 1;
          if (Times_Hour_Start[pos]==24) Times_Hour_Start[pos]=0;
          lcd.clear();
          }
        }
      else if (enc.hold() and var%2==1) {
        delay(100);
        Times_Minute_Start[pos] -= 1;
        if (Times_Minute_Start[pos]==0){
          Times_Minute_Start[pos]=59;
          Times_Hour_Start[pos] -= 1;
          if (Times_Hour_Start[pos]==-1) Times_Hour_Start[pos]=23;
          lcd.clear();
        }
      }

      //Обработчик изменения направления изменения времени начала
      if (enc.click()) var+=1;

      //Если 2 клика, то меняется намер канала для включения по времени именно в этом Option
      if (enc.hasClicks(2)) Chanel[pos] = (Chanel[pos]+1)%Max_Chanel;
    } 
    if (list == 1){
      lcd.setCursor(0, 0);
      lcd.print("Days   ");
      if (Days_watering[pos][0]){
        if (pos_day == 0) lcd.print("*M<");
        else lcd.print("*M ");
      } 
      else  {
        if (pos_day == 0) lcd.print(" M<");
        else lcd.print(" M ");
      }
      if (Days_watering[pos][1]){
        if (pos_day == 1) lcd.print("*T<");
        else lcd.print("*T ");
      } 
      else  {
        if (pos_day == 1) lcd.print(" T<");
        else lcd.print(" T ");
      }
      if (Days_watering[pos][2]){
        if (pos_day == 2) lcd.print("*W<");
        else lcd.print("*W ");
      } 
      else  {
        if (pos_day == 2) lcd.print(" W<");
        else lcd.print(" W ");
      }
      lcd.setCursor(0, 1);
      if (Days_watering[pos][3]){
        if (pos_day == 3) lcd.print("*Th<");
        else lcd.print("*Th ");
      } 
      else  {
        if (pos_day == 3) lcd.print(" Th<");
        else lcd.print(" Th ");
      }
      if (Days_watering[pos][4]){
        if (pos_day == 4) lcd.print("*Fr<");
        else lcd.print("*Fr ");
      } 
      else  {
        if (pos_day == 4) lcd.print(" Fr<");
        else lcd.print(" Fr ");
      }
      if (Days_watering[pos][5]){
        if (pos_day == 5) lcd.print("*Sa<");
        else lcd.print("*Sa ");
      } 
      else  {
        if (pos_day == 5) lcd.print(" Sa<");
        else lcd.print(" Sa ");
      }
      if (Days_watering[pos][6]){
        if (pos_day == 6) lcd.print("*Su<");
        else lcd.print("*Su ");
      } 
      else {
        if (pos_day == 6) lcd.print(" Su<");
        else lcd.print(" Su ");
      }


      //Обработчик поворота при нажатии. Навигация по дням
      if (enc.rightH()) {
        if (pos_day<7) pos_day ++;
        }
      else if (enc.leftH()) {
        if (pos_day>0) pos_day --;
        } 
      
      if (enc.isClick()){
        if (Days_watering[pos][pos_day] == 0) Days_watering[pos][pos_day]=1;
        else Days_watering[pos][pos_day] = 0;
      }
    } 

    
    //Обработчик нажатия и переход между Options
    if (enc.hasClicks(5) and pos < Len_Menu) {
      pos += 1;
      list = 0;
      lcd.clear();
      }
    else if (enc.hasClicks(6) and pos > 0) {
      pos -= 1;
      list = 0;
      lcd.clear();
      } 
    
    //Обработчик поворота и перехода внутри Option
    if (enc.right() or enc.left()){
      lcd.clear();
      if (list == 1) list=0;
      else list=1;
    }
  }

  //Обработчик перехода между основным экраном и настройками
  if (enc.hasClicks(4)) {
    Print_time = not(Print_time);
    lcd.clear();
  }
  //Обработчик перехода между основным экраном и настройками времени
  else if (enc.hasClicks(5)){
    settings_time = not(settings_time);
    lcd.clear();
  }

  //Цыкл для включения в определеннное время
  for (int i=0;i<Len_Menu;i++){
    if ((HOUR*60+MIN) - (Times_Hour_Start[i]*60+Times_Minute_Start[i]) == 0 and SEC == 0 and Days_watering[i][DAY]){
      Sost_Chanel[Chanel[i]] = 1;
    }
    if ((HOUR*60+MIN) - (Times_Hour_Start[i]*60+Times_Minute_Start[i]) == Timer[i] and SEC == 0){
      Sost_Chanel[Chanel[i]] = 0;
    }
  }

  //Включение реле
  for (int q=0;q<Max_Chanel;q++){
    if (Sost_Chanel[q] == 1){
      digitalWrite(q+Relay_first, HIGH);
    }
    else {
      digitalWrite(q+Relay_first, LOW);
    }
  }
}

