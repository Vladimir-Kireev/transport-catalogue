<h1 align="center">Транспортный справочник / роутер</h1>
<h3 align="center"><img src="https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white"> <img src="https://img.shields.io/badge/Visual%20Studio-5C2D91.svg?style=for-the-badge&logo=visual-studio&logoColor=white"> <img src="https://img.shields.io/badge/git-%23F05033.svg?style=for-the-badge&logo=git&logoColor=white"></h3>
<h3>Система хранения транспортных маршрутов и обработки запросов к ней:</h3>
<ul>
  <li>Входные данные и ответ в JSON-формате;</li>
  <li>Выходной JSON-файл может содержать визуализацию карты маршрута(ов) в формате SVG-файла;</li>
  <li>Поиск кратчайшего маршрута;</li>
  <li>Сериализация базы данных и настроек справочника при помощи Google Protobuf;</li>
  <li>Объекты JSON поддерживают цепочки вызовов (method chaining) при конструировании, превращая ошибки применения данных формата JSON в ошибки компиляции.</li>
</ul>
<h3>Использованные технологии:</h3>
<ul>
  <li>OOP: inheritance, abstract interfaces, final classes;</li>
  <li>JSON load / output;</li>
  <li>SVG image format embedded inside XML output;</li>
  <li>Curiously Recurring Template Pattern (CRTP);</li>
  <li>Method chaining;</li>
  <li>C++17 with С++20 Ranges emulation.</li>
</ul>
