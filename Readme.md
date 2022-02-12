Nx,(B7M5e4X/PBp
Прошивка контроллера открытия/закрытия ворот.
1. Устройство перманенто в самом глубоком сне.
2. При срабатывании радио сигнал VT заведен на INT1. который будит контроллер и тот делает работу.
3. Manualkey заведен на INT0.
4. Прерывания INT0, INT1 устанавливают желаемые настройки ворот, будят контроллер.
5. В основном цикле выволняется управление воротами и затем снова спать.
6. Несмотря на то, что WatchDog в Atmega8 имеет отдельный клок, он не умеет генерировать прерывание, а может только ребутнуть процессор, когда выйдет время. У большинства прочих AVR контроллеров WatchDog имеет вектор прерывания и помимо INT0, INT1 можно будеться по собаке, чтобы проверить не зависли ли, обновить время и снова уснуть. Так как в Atmega8 этого нет, WatchDog вообще придется отключить.
7. Если оставить WatchDof контроллер будет ребутаться. Вприципе можно сделать так, что не будет важным, контроллер всегда будет приводиться в тоже самое начальное состояние. Но неизвестно что будет происзодить с входами/выходами во время ребута. Поэтому, скорее всего такой вариант плохой.
8. ИК датчик должен останавливать ворота только во время  


To build app tasks used. Ctrl+P -> task Build ot task Clean.
Task creates shell with make instance. Makefile is minimalistic to produce firmware by avr-gcc.
To make avr-gcc work need to add path (systempropertiesadvanced.exe)
http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_port_pass


https://alexgyver.ru/lessons/compute/
https://crccalc.com/
https://nongnu.org/avr-libc/user-manual/
https://blog.podkalicki.com/100-projects-on-attiny13/
https://stackoverflow.com/questions/24343911/convert-integer-to-char-array-function
https://ravesli.com/uroki-po-qt5/