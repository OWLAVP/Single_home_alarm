#define DDR_W       DDRB
#define PIN_W       PINB
#define PIN_N       0

#define OWIRE_0 DDR_W |= (1<<PIN_N)			// Прижимаем линию к GND
#define OWIRE_1 DDR_W &= ~(1<<PIN_N)			// Отпускаем линию и даем подтянуть к питанию через резистор

#include "main.h"

unsigned char owire_init()					// Функция инициализации. Возвращает 1 если присктствует устройство на шине, иначе 0
{
    cli();									// Запрещаем прерывания на время работы функции, дабы те не вмешались во временные задержки. Можно убрать если не использовать прерывания
    unsigned char i;						// Переменная для цикла
    OWIRE_0;								// Прижимаем линию к GND
    _delay_us(480);							// Топчимся 480 мкс
    OWIRE_1;								// Отпускаем линию
    _delay_us(2);							// Тпчимся 2 мкс

    for (i=0; i<100; i++)					// Ждём не менее 60 мкс до появления импульса присутствия. (Линия должна прижаться Слейвом к GND)
    {
        if (!(PIN_W & (1<<PIN_N)))			// Читаем уровень на линии. Если 0 то бежим в цикл ниже
        {
            while (!(PIN_W & (1<<PIN_N)));	// Если проишел импульс присутствия, ждём его окончания. То есть ждем 1
            sei();							// Разрешаем прерывания. Мы же их запрещали. Так же можно выкинуть от сюда
            return 1;						// Возвращаем 1 если хоть одно устройство присутствует на шине
        }
        _delay_us(1);						// Немного потупим
    }
    sei();									// Разрешаем прерывания. Это если устройств не обнаружено, то мы в тело ИФа не зайдем.
    return 0;								// Возвращаем 0, устройств на шине нэма.
}


void owire_write(unsigned char b)			// Функция для передачи байта в шину
{
    cli();									// Запрещаем прерывания. Чтоб не мешали.
    unsigned char temp, i;					// Создаем буферную переменную и переменную для цикла
    for(i=0; i<8; i++)						// Восемь раз выполняем код
    {
        temp=(b&0x01);						// В буфер пихаем значение передаваемого байта логически умноженного на 1. Короче вычленяем значение бита нулевого разряда.
        if(temp)							// Если этот бит равен 1 то пихаем 1 в шину. Делается это так
        {
            OWIRE_0;						// Сначала прижимаем линию к GND
            _delay_us(6);					// Затем удерживаем 6 мкс (Так рекомендует тех документация которай у меня есть по шине 1-wire)
            OWIRE_1;						// Теперь отпускаем линию, пусть резистор работает
            _delay_us(64);					// Выжидаем в таком состоянии 64 мкс.
        }
        else								// Если бит был равен 0
        {
            OWIRE_0;						// Прижимаем линию к GND
            _delay_us(60);					// Ждем 60 мкс
            OWIRE_1;						// Отпускаем линию
            _delay_us(10);					// Ждем 10 мкс
        }
        b>>=1;								// Двигаем вправо наш передоваемый байт на 1 разряд дабы передать следующий байт
    }
    sei();									// Разрешаем прерывания
}

unsigned char owire_read_bit()				// Функция чтения бита из шины. По сути эту функцию можно интегрировать в функцию чтения байта. Я ее создал для чтения уровня, но не использую
{
    cli();									// Запрещаем прерывания
    unsigned char bit;						// Переменная в которой будет хрониться	бит
    OWIRE_0;								// Прижимаем линию к GND
    _delay_us(2);							// Ждем 2 мкс
    OWIRE_1;								// Отпускаем линию
    _delay_us(5);							// Ждем 5 мкс
    bit = PIN_W & (1<<PIN_N);					// Читаем уровень на линии и записываем значение в переменную
    _delay_us(80);							// Ждем 80 мкс. Длинна передачи бита
    sei();									// Разрешаем прерывания
    return bit;								// Возвращаем значение бита
}


unsigned char owire_read()					// Функция чтения байта
{
    cli();									// Запрещаем прерывания
    unsigned byte = 0, i;					// Создаем переменные буфера(обязательно обнулить, а то запишется мусор) и для цикла
    for (i=0; i<8; i++)						// Условие цикла
    {
        byte >>= 1;							// Двигаем бит по нашему байту вправо
        if (owire_read_bit())				// Читаем бит. Вот здесь можно читать уровень, а битовую функцию выкинуть. Лишний код етит
            byte |= 0x80;						// Логическое или с нашим байтом. Если пришла 1, то запишем ее
    }
    sei();									// Разрешить прерывания
    return byte;							// Возвращаем принятый байт
}

int8_t get_18x20_temperature(void)
{
    unsigned char temp[2];							// Массив для получения данных из датчика. Смешно, да. Вообще он должен быть на 9 ячеек, так как память датчика содержит 9 байт
    int8_t temper;
    owire_init();							// Сброс датчика
    owire_write(0xCC);						// Без РОМ кода. Широковещательная команда
    owire_write(0x44);						// Команда начать преобразование температуры
    // Тупим пока датчик пыхтит с преобразованием и укладки данных к себе в память
    while (!(PIN_W & (1<<PIN_N))){;};
    owire_init();							// Сброс датчика.
    owire_write(0xCC);						// Опять орем на всех сразу
    owire_write(0xBE);						// Команда передать данные от датчика к мастеру

    temp[0]=owire_read();					// Читаем младший байт температуры
    temp[1]=owire_read();					// Читаем старший байт температуры

    if(temp[1]&0x80)						// Если температура минусовая
    {
        temp[0] = (~temp[0])+1;				// Побитное или + 1
        temp[1] = ~temp[1];					// Побитное или
    }

    temper = (int8_t) (((temp[1]<<8)|(temp[0]*5))/10);	// Эта абра-кадабра кладет в переменную temper значение температуры умноженной на 10. Например 23,5 градусов будут в виде 235
    return temper;
}
