#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <EncButton.h>
EncButton<EB_TICK, 2, 3, 4> enc;

#include "iarduino_RTC.h"
iarduino_RTC time(RTC_DS1302,8,6,7); 

#include "GyverTimer.h"

int Relay_first = 9; //Первое реле. Потом подключаем в каждый последующий выход по реле(не пропуская)
int Len_Menu = 3 - 1; //Длинна меню меняем первую цыфру

int Max_Chanel = 4 + 1; //Максимум каналов
int Sost_Chanel[] = {0, 0, 0, 0}; //Состояния каналов 0 или 1 (вкл или выкл)

int Menu[] = {1,2,3};//Номера Option
int Times_Hour_Start[] = {1,10,13};//Время часы
int Times_Minute_Start[] = {50,41,47};//Время минуты
int Timer[] = {10, 1, 12};//Таймер
int Chanel[] = {0, 0, 2};//Номер канала привязанный к Option

int pos = 0;//Позиция в настройках
int pos_first = 0;//Позиция на начальном экране 
int Pos_CH = 1;//Позиция на экране всех каналов
int var = 0;//Регулировка начала регулировки времени старта полива (увеличение или уменьшение) 0 или 1

bool Vkl = false;
bool Print_time = true; //Выводить начальный экран или настройки Options

void setup() {
  lcd.init(); // инициализация
  lcd.backlight(); //Включить подстветку экрана

  Serial.begin(9600);
  pinMode(Chanel[pos]+Relay_first, OUTPUT);//Говорим что пин, к которому подключено реле выдает напряжение

  time.begin();
}

void loop() {
  //Для использования инкодера
  enc.tick();

  //Начальный экран, если начать 4 раза, то можно перейти в настройки автополива по времени
  if (Print_time) {

    //Начальный экран времени
    if ((pos_first%2) == 0){
      lcd.setCursor(3, 0);
      lcd.print(time.gettime("d-m-Y"));
      lcd.setCursor(4, 1);
      lcd.print(time.gettime("H:i:s"));
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
    
    //Экран с каналами. Можно раками включить определенный канал
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



  //Если нажать 4 раза, то появится меню настройки таймеров включения полива
  else{
    //pos сразу же выбирает какой полив смотрим. Потом про помощи прокрутки меняется

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

    //Обработчик поворота и переход между Options
    if (enc.right() and pos < Len_Menu) {
      pos += 1;
      lcd.clear();
      }
    else if (enc.left() and pos > 0) {
      pos -= 1;
      lcd.clear();
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

  //Обработчик перехода между основным экраном и настройками
  if (enc.hasClicks(4)) {
    Print_time = not(Print_time);
    lcd.clear();
  }

  //Цыкл для включения в определеннное время
  for (int i=0;i<Len_Menu;i++){
    if ((time.Hours*60+time.minutes) - (Times_Hour_Start[i]*60+Times_Minute_Start[i]) == 0 and time.seconds == 0){
      Sost_Chanel[Chanel[i]] = 1;
    }
    if ((time.Hours*60+time.minutes) - (Times_Hour_Start[i]*60+Times_Minute_Start[i]) == Timer[i] and time.seconds == 0){
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

