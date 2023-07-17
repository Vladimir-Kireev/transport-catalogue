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
  <li>std::variant and std:optional;</li>
  <li>SVG image format embedded inside XML output;</li>
  <li>Static libraries .LIB/.A ;</li>
  <li>Directed Weighted Graph data structure for Router module;</li>
  <li>Google Protocol Buffers for data serialization;</li>
  <li>Curiously Recurring Template Pattern (CRTP);</li>
  <li>Method chaining;</li>
  <li>CMake generated project and dependency files;</li>
  <li>C++17.</li>
</ul>
<h3>Запуск программы</h3>
Приложение транспортного справочника спроектировано для работы в 2 режимах: режиме создания базы данных и режиме запросов к базе данных.

Для создания базы данных транспортного справочника с последующей ее сериализацией в файл необходимо запустить программу с параметром make_base. Входные данные поступают из stdin, поэтому можно переопределить источник данных, например, указав входной JSON-файл, из которого будет взята информация для наполнения базы данных вместо stdin.
<br>Пример: `transport_catalogue.exe make_base <input_data.json`

Для обработки запросов к созданной базе данных (сама база данных десериализуется из ранее созданного файла) необходимо запустить программу с параметром process_requests, указав входной JSON-файл, содержащий запрос(ы) к БД и выходной файл, который будет содержать ответы на запросы, также в формате JSON.
<br>Пример: `transport_catalogue.exe process_requests <requests.json >output.txt`
<h3>Формат входных данных</h3>
Входные данные принимаются из stdin в JSON формате. Структура верхнего уровня имеет следующий вид:
<pre>
{
  "base_requests": [ ... ],
  "render_settings": { ... },
  "routing_settings": { ... },
  "serialization_settings": { ... },
  "stat_requests": [ ... ]
}
</pre>
<br>Каждый элемент является словарем, содержащим следующие данный:
<br><code>base_requests</code> — описание автобусных маршрутов и остановок;
<br><code>stat_requests</code> — запросы к транспортному справочнику;
<br><code>render_settings</code> — настройки рендеринга карты в формате .SVG;
<br><code>routing_settings</code> — настройки роутера для поиска кратчайших маршрутов;
<br><code>serialization_settings</code> — настройки сериализации/десериализации данных.
