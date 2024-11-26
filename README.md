# Большое домашнее задание 1
# In-memory Database

## Описание

База данных (файл ```database.h```) состоит из нескольких именованных таблиц. 
Каждая таблица (файл ```table.h```) состоит из списка описателей столбцов (структура ```Column``` в файле ```column.h```),
который задается при создании новой таблицы.
Данные хранятся в виде непрерывной неструктурированной области памяти (```uint8_t *storage```).
Адреса строк таблицы вычисляются как

```C++
uint8_t *row_ptr = storage + row_index * row_size;
```

Параметр ```row_size``` вычисляется при создании таблицы на основе данных из списка описателей столбцов.
Каждый столбец имеет тип и соответствующий размер (int32 - 4 байта, bool - 1 байт, длина строкового и 
байтового типов задается явно в описателе столбца). ```row_size``` является суммой размеров всех столбцов.
Описатель столбца (структура ```Column```) помимо прочего содержит поле ```offset```, которое содержит
смещение столбца от начала строки. ```offset``` для каждого столбца вычисляется при создании таблицы 
вместе с вычислением ```row_size``` (как сумма размеров предыдущих столбцов). 
Тогда адреса полей в строке таблицы вычисляются как

```C++
uint8_t *value_ptr = row_ptr + column_offset;
```

Для чтения/записи значений из таблицы выполняется преобразование этого указателя в соответствующий тип.

Память для данных выделяется с запасом, для ускорения операций добавления новых строк. Стратегия выделения 
памяти похожа на ту, которая используется в ```std::vector``` - при исчерпании места под данные текущая 
емкость удваивается, создается новая область памяти удвоенного размера и данные копируются туда.

Интерфейс базы данных, в целом, аналогичен примеру из задания:

```C++
#include <memdb.h>
memdb::Database db;
db.load_from_file(std::ifstream("db.bin", ios::bin));
auto result = db.execute(query);
if (result.is_ok()) {
    for (auto &row : result) {
        int i = resp.get<int>("column1");
        std::string s = resp.get<std::string>("column2");
        do_something(i, s);
    }
} else {
    std::cerr << "Error: " << result.get_error() << "\n";
}
db.save_to_file(std::ofstream("db.bin", ios::bin));
```

Но. ```execute(query)``` пока реализованно только для ```"create table ..."```, ```"create ... index ..."``` и ```"insert ..."```.


Запрос выборки данных (select) реализован пока только в очень упрощенной форме и только через метод класса ```Database::select(conditions)```. 
```conditions``` в этом методе представляет собой список условий, каждое из которых состоит из индекса столбца, 
к которому будет применено условие, одной из шести операций сравнения и значения, с которым будет сравниваться значения из столбца таблицы.
Все условия объединены логической операцией "И", то есть, чтобы строка попала в выборку должны быть выполнены все условия. 
Таким образом можно задать только очень ограниченные условия выборки, например:

```
a >= 100 && a < 200 && a != 150 && b == true
```

Запросы обновления (update) и удаления (delete) данных пока не реализованны ни в какой форме.

Реализовано создание ordered-индексов, но unordered-индексы пока не реализованы.

ordered-индексы реализованы, но не до конца. Необходимо еще доделать их, чтобы индексы обновлялись при добавлении/обновлении/удалении значений.
ordered-индекс представляет собой массив индексов строк, упорядоченный по значениям заданного столбца. Имея упорядоченный массив, мы можем
выполнять быстрый бинарный поиск по столбцу, что позволяет быстро найти кандитатов на выборку. Например, у нас есть условие 
```a >= 100 && a < 200```. Без использования индекса придется проверить каждую строку таблицы на соответствие заданному условию. 
При использовании индекса с помощью бинарного поиска находится нижняя и верхняя граница диапазона выборки, что может значительно 
сократить количество проверяемых строк. Чем уже полученный диапазон, тем быстрее будет проходить выборка. Если же диапазон выборки 
получается слишком широким (вплоть до того, что в него входят все строки таблицы), то в этом случае выборка может работать медленнее, 
чем без использования индексов, из-за дополнительных накладных расходов. Если имеется несколько индексов по разным колонкам, то используется 
только один, тот, который дает более узкий диапазон. Тестирование показало значительное ускорение выборки при использовании индексов.
Например, в таблице из миллиона строк, выборка из примерно 300 записей без использования индекса занимет примерно 20 мс, а с использованием
индекса - 0 мс.

Каждый запрос к базе данных приводит к возращению структуры типа ```ResultSet``` (файл ```resultset.h```).
Эта структура содержит результат запроса (true или false), сообщение об ошибке, если запрос закончился неудачей, а также время 
выполнения запроса в миллисекундах. При запросе выборки данных (select) структура содержит некоторое количество выбранных строк,
по которым можно итерироваться с помощью forward-итератора ```ResultSetIterator```. Организация данных, содержащихся в результате выборки, 
аналогична той, которая используется в самой таблице, то есть, эти данные представляют собой непрерывную неструктурированную область памяти.
Разыменованный ```ResultSetIterator``` возвращает объект типа ```ResultRow``` (файл ```resultrow.h```), который позиционируется на определенную
строку выборки. ```ResultRow``` имеет шаблонный метод ```get(name)```, который аозволяет получить значение из строки по указанному имени столбца.

## Сборка и тестирование

Используется система сборки CMake.

Библиотека состоит только из заголовочных файлов, находящихся в каталоге ```lib/memdb/include```. Для подключения библиотеки к проекту 
достаточно добавить это каталог в include directories проекта и подключить заголовочный файл ```<memdb.h>```.

Для тестирования используется ```gtest```. В данный момент реализованно только тестирование лексического анализатора, используемого при
разборе текстовых запросов. Для запуска тестов нужно перейти в каталог ```lib/memdb``` и выполнить следующие команды:

```
mkdir build
cd build
cmake ..
make
ctest
```

В корневой директории находится демонстрационный проект, в котором все реализованные вещи проверяются на практике. Код находится в файле 
src/driver.cpp, компилируется в исполняемый файл driver. Для сборки и запуска необходимо выполнить следующие команды в корневом каталоге:

```
mkdir build
cd build
cmake ..
make
./driver
```

Результат выполнения будет примерно таким:

```
Populating table... 4358 ms

Creating ordered index on the first column... 138 ms

Creating ordered index on the second column... 1213 ms

Saving to file... 164 ms

Loading from file... 153 ms

Database info:
1: users (4 columns, 1000000 rows)

Selecting all... 16 ms (1000000 rows)

Selecting by conditions... 0 ms (250 rows)

id      login           is_admin code
10047   b               1        0x2708e1c2
10227   azzlmmydm       1        0x821382ec
10305   b               1        0x54658119
10355   cmvhlgwdkznpx   1        0x68fbfe97
...........................................
...........................................
...........................................
```

Также будет создан файл базы данных db.bin размером примерно 40 МБ.
