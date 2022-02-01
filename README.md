# Mp3 file meta information editor
###### Лабораторная работа по программированию №4

Реализация редактора текстовой метаинформации mp3 файла.  
В качестве стандарта метаинформации принят ID3v2.  

Редактор представляет из себя консольную программу принимающую в качестве  аргументов имя файла через параметр __--filepath__ , а также одну из выбранных команд:  
1. __--show__ - отображение всей метаинформации в виде таблицы  
2. __--set=prop_name --value=prop_value__ - выставляет значение определенного поля метаинформации с именем prop_name в значение prop_value  
3. __--get=prop_name__ - вывести определенное поле метаинформации с именем prop_name


#### Название файла и опции передаются через аргументы командной строки в следующем формате:
___Editor_mp3.exe --filepath=[filename] [OPTIONS]___

---
#### Примеры использования программы:  
1. ___Входные аргументы:___ Editor_mp3.exe --filepath=file_example_MP3_700KB.mp3 --show  
___Выходные данные:___  
ID3v2 header  
file identifier: ID3  
version: 3.0  
flags: 0  
size: 127  
\
Frames  
frame id: TCON  
size: 10  
flags: 0 0  
text encoding: 0  
information: Cinematic  
\
frame id: TALB  
size: 22  
flags: 0 0  
text encoding: 0  
information: YouTube Audio Library  
\
frame id: TIT2  
size: 9  
flags: 0 0  
text encoding: 0  
information: my music  
\
frame id: TPE1  
size: 14  
flags: 0 0  
text encoding: 0  
information: Kevin MacLeod
2. ___Входные аргументы:___ Editor_mp3.exe --filepath=file_example_MP3_700KB.mp3 --set=TIT2 --value="Test music"  
___Выходные данные:___  Программа ничего не выводит. Значение поля TIT2 в файле example_MP3_700KB.mp3 заменено на "Test music".
3. ___Входные аргументы:___ Editor_mp3.exe --filepath=file_example_MP3_700KB.mp3 --get=TIT2  
___Выходные данные:___  
frame id: TIT2  
size: 11  
flags: 0 0  
text encoding: 0  
information: Test music