# main file

from data_processing import *
from ui import *
from kaspersky_api import *
import sys

def main():
    app = QApplication(sys.argv)                # Создаем приложение
    splash = SplashScreen(duration=1000)        # Время отображения заставки в миллисекундах
    if splash.exec() == QDialog.DialogCode.Accepted:
        main_window = MainWindow()
        main_window.show()                      # Показываем основное окно
    sys.exit(app.exec())                        # Запускаем приложение

if __name__ == "__main__":
    main()