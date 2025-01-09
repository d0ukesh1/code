from PyQt6.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QApplication, QLabel, QTabWidget, QPushButton, QHBoxLayout, QFrame, QTabBar, QTextEdit, QMenu, QMessageBox, QStyle
from PyQt6.QtGui import QIcon
from PyQt6.QtCore import Qt, QSize
from PyQt6 import QtCore, QtGui, QtWidgets

from .gui_tab_1 import Tab1Content
from .gui_tab_2 import Tab2Content
import os
import sys


class TabBar(QTabBar):
    def tabSizeHint(self, index):
        size = QTabBar.tabSizeHint(self, index)
        w = int(self.width() / self.count())  # Равномерное распределение ширины вкладок
        return QSize(w, size.height())

def resource_path(relative_path):
    """ Получает абсолютный путь к ресурсам, которые добавлены в .exe. """
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return relative_path

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(650, 325)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")

        # Создаем QTabWidget и настраиваем его
        self.tabWidget = QTabWidget(self.centralwidget)
        self.tabWidget.setObjectName("tabWidget")
        self.tabWidget.setContextMenuPolicy(QtCore.Qt.ContextMenuPolicy.CustomContextMenu)  # Разрешаем контекстное меню
        self.tabWidget.customContextMenuRequested.connect(self.show_context_menu)  # Связь с функцией

        # Используем TabBar с кастомным размером вкладок
        self.tabWidget.setTabBar(TabBar())                                
        self.tabWidget.setTabsClosable(False)
        self.tabWidget.setMovable(False)
        self.tabWidget.setDocumentMode(True)
        self.tabWidget.setElideMode(Qt.TextElideMode.ElideRight)
        self.tabWidget.setUsesScrollButtons(True)
        
        # Создаем вкладки
        self.tab = Tab1Content()
        self.tabWidget.addTab(self.tab, "")
        
        self.tab_2 = Tab2Content()
        self.tabWidget.addTab(self.tab_2, "")
        
        # Добавляем QTabWidget в основной макет
        self.gridLayout.addWidget(self.tabWidget, 0, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        
        # Статусная строка
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def show_context_menu(self, position):
        """Показывает контекстное меню для вкладок."""
        tab_index = self.tabWidget.tabBar().tabAt(position)
        
        menu = QMenu()
        # Получаем предустановленную иконку вопроса
        question_icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxQuestion)
        # Добавляем действие справки и привязываем к текущему индексу вкладки
        help_action = menu.addAction(question_icon, "Справка")
        help_action.triggered.connect(lambda: self.show_help_message(tab_index))

        # Показываем меню в позиции курсора
        menu.exec(self.tabWidget.mapToGlobal(position))

    def show_help_message(self, tab_index):
        """Показывает сообщение со справкой для каждой вкладки."""
        help_text1 = """
        - Шаг 1. Выберете файл "Проверка", в котором содержится лист с экспортированным списком устройств, у которых в реестре программ были обнаружены некорпоративные браузеры.
        - Шаг 2. Выберете файл, в котором содержится таблица с именами устройств. Важно, чтобы был столбец "ИМЯ ПК" и имя файла содержало дату в формате "ГГГГ-ММ-ДД" или "ГГГГ_ММ_ДД". На этом шаге будут созданы дополнительные столбцы и таблица будет отформатирована к единому оформлению, также созданы файлы "ГГГГ-ММ-ДД_MIR.txt", "ГГГГ-ММ-ДД_MSK.txt" и "ГГГГ-ММ-ДД_Проверка.txt". В последнем файле находятся имена ПК, которые были обнаружены в файле "Проверка".
        - Шаг 3. Выберете тот же файл, что и на Шаге 2. На этом этапе программа предложит заполнить конкретным именам ПК указанные в программе поля, такие как статус устройства и обнаруженный браузер. Важно, что программа автоматически заполнит все устройства полями "ОК", так что требуется указать только проблемные названия из выпадающего списка.
        """
        help_text2 = """
        - Шаг 1. Для начала работы нужно заполнить данные в полях URL, Login, Password, а также выбрать файл, в котором содержится таблица с именами устройств. Важно, чтобы был столбец "ИМЯ ПК" и имя файла содержало дату в формате "ГГГГ-ММ-ДД" или "ГГГГ_ММ_ДД".
        - Шаг 2. Нажмите на кнопку "Авторизоваться и Обработать файл". Программа попытается подключиться к серверу KSC по введеным логину и паролю.
        - Шаг 3. Ждите результат работы программы. Будут получены данные об устройствах из указанного файла, обработаны и записаны в таблицу.
        """
        if tab_index == 0:
            QMessageBox.information(self, "Руководство по работе без подключения к серверу", help_text1)
        elif tab_index == 1:
            QMessageBox.information(self, "Руководство по работе с подключением к серверу", help_text2)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "Alrosa & KSC | Excel master"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), _translate("MainWindow", "Без подключения к серверу"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), _translate("MainWindow", "С подключением к серверу"))


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        icon_path = resource_path("ui/alrosa.ico")
        self.setWindowIcon(QIcon(icon_path))
        self.setupUi(self)
        