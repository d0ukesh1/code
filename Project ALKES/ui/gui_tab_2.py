from PyQt6.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QLabel, QLineEdit, QPushButton, QMessageBox, QFileDialog
from PyQt6 import QtCore
import os
from data_processing import *
from kaspersky_api import *

class Tab2Content(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.filename = None
        self.editor = None
        self.setup_ui()
        self.load_settings()

    def resizeEvent(self, event):
        """Обрабатываем изменение размера окна"""
        super().resizeEvent(event)

        # Обновляем размеры полей ввода при изменении размера окна
        field_width = self.width() // 2 - 14
        self.url_edit.setFixedWidth(field_width)
        self.login_edit.setFixedWidth(field_width)
        self.password_edit.setFixedWidth(field_width)
    
    def setup_ui(self):
        """Настройка интерфейса без отступа сверху"""
        # Основной макет
        self.verticalLayout = QVBoxLayout(self)
        
        # Поле ввода URL
        self.url_label = QLabel("URL:", self)
        self.url_edit = QLineEdit(self)
        
        # Расположение URL на вертикальном лейауте
        self.horizontalLayout1 = QHBoxLayout()
        self.horizontalLayout1.addWidget(self.url_label)
        self.horizontalLayout1.addWidget(self.url_edit)
        self.verticalLayout.addLayout(self.horizontalLayout1)

        # Поле ввода Login
        self.login_label = QLabel("Login:", self)
        self.login_edit = QLineEdit(self)

        # Расположение Login на вертикальном лейауте
        self.horizontalLayout2 = QHBoxLayout()
        self.horizontalLayout2.addWidget(self.login_label)
        self.horizontalLayout2.addWidget(self.login_edit)
        self.verticalLayout.addLayout(self.horizontalLayout2)

        # Поле ввода Password
        self.password_label = QLabel("Password:", self)
        self.password_edit = QLineEdit(self)
        self.password_edit.setEchoMode(QLineEdit.EchoMode.Password)
        
        # Расположение Password на вертикальном лейауте
        self.horizontalLayout3 = QHBoxLayout()
        self.horizontalLayout3.addWidget(self.password_label)
        self.horizontalLayout3.addWidget(self.password_edit)
        self.verticalLayout.addLayout(self.horizontalLayout3)

        # Выбор файла Excel
        self.file_label = QLabel("Выберите Excel-файл, который нужно обработать",self)
        self.file_label.setWordWrap(True)
        self.file_button = QPushButton("Выбрать")
        self.file_button.clicked.connect(self.file_dialog)

        # Расплоложение выбора файла на вертикальном лейауте
        self.horizontalLayout4 = QHBoxLayout()
        self.horizontalLayout4.addWidget(self.file_label)
        self.horizontalLayout4.addWidget(self.file_button)
        self.verticalLayout.addLayout(self.horizontalLayout4)

        # Кнопка для авторизации
        self.login_button = QPushButton(" Авторизоваться и Обработать файл ", self)
        self.login_button.clicked.connect(self.authenticate)

        # Расположение кнопки для авторизации
        self.button_layout = QHBoxLayout()
        self.button_layout.addStretch()
        self.button_layout.addWidget(self.login_button)
        self.button_layout.addStretch()
        self.verticalLayout.addLayout(self.button_layout)

        self.verticalLayout.addStretch()

    def load_settings(self):
        """Загружает URL и логин, если они сохранены в настройках."""
        settings = QtCore.QSettings("Alrosa", "KSC_auth")
        saved_url = settings.value("url", "")
        saved_login = settings.value("login", "")
        self.url_edit.setText(saved_url)
        self.login_edit.setText(saved_login)

    def save_settings(self):
        """Сохраняет URL и логин в настройки."""
        settings = QtCore.QSettings("Alrosa", "KSC_auth")
        settings.setValue("url", self.url_edit.text())
        settings.setValue("login", self.login_edit.text())

    def authenticate(self):
        """Обрабатывает процесс аутентификации."""
        url = self.url_edit.text()
        login = self.login_edit.text()
        password = self.password_edit.text()
        
        # Проверка данных
        if self.check_credentials(url, login, password, self.filename):
            self.save_settings()
            api = O_API(server_url = url, username=login, password=password)
            if api.server:  # Проверяем, что соединение с сервером установлено
                try:
                    hostnames = list(Computer.load_computers(self.filename).keys())
                    Logger.log_info(f"Компьютеры - {hostnames} - отправлены на проверку.")
                    Logger.log_info(f"Начнем проверку для {hostnames[0]}")
                    hostname0 = api.FindHostByDN(hostnames[0])
                    Logger.log_info(f"Данные FindHostByDN : {hostname0}")
                    output = api.GetHostInfo(hostname0["KLHST_WKS_HOSTNAME"])
                    Logger.log_info(f"Данные об устройстве {hostnames[0]} успешно получены : {output}")
                    QMessageBox.information(self, "Успех", "Удалось подключиться к серверу. Данные о устройстве были получены.")
                    self.close()
                except Exception as e:
                    Logger.log_error(f"Ошибка при получении данных: {str(e)}")
                    QMessageBox.warning(self, "Упс!", "Что-то пошло не так, проверьте файл лога.")
            else:
                QMessageBox.warning(self, "Ошибка", "Не удалось установить соединение с сервером.")
    
    def check_credentials(self, url, login, password, filename):
        """Проверяет правильность URL, логина и пароля."""
        
        # Проверка URL
        url_pattern = r"^https:\/\/[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}:\d+$"
        if not re.match(url_pattern, url):
            Logger.log_error("Некорректный формат URL. Ожидаемый формат: https://machine.domain:port")
            QMessageBox.warning(self, "Ошибка", "Некорректный формат URL. Ожидаемый формат: https://machine.domain:port")
            return False

        # Проверка логина
        if not login:
            Logger.log_error("Логин не может быть пустым.")
            QMessageBox.warning(self, "Ошибка", "Логин не может быть пустым.")
            return False

        # Проверка пароля
        if not password:
            Logger.log_error("Пароль не может быть пустым.")
            QMessageBox.warning(self, "Ошибка", "Пароль не может быть пустым.")
            return False

        if not filename:
            Logger.log_error("Файл для обработки не выбран.")
            QMessageBox.warning(self, "Ошибка", "Файл для обработки не выбран.")
            return False
        # Если все проверки пройдены, вернуть True
        return True

    def file_dialog(self):
        """ Открывает диалог для выбора файла. """
        try:
            self.filename, _ = QFileDialog.getOpenFileName(self, "Выбрать Excel файл", "", "Excel Files (*.xlsx; *.xls);;All Files (*)")
            
            if self.filename:
                Logger.log_info(f"Файл {self.filename} выбран.")
                self.file_label.setText(f"Файл выбран: {os.path.basename(self.filename)}")
            else:
                Logger.log_error("Файл для обработки не выбран.")
                self.file_label.setText("Файл для обработки не выбран.")
            
            # Извлечение даты из имени файла
            match = re.search(r"\d{4}[-_]\d{2}[-_]\d{2}", self.filename)
            extract_date_value = match.group(0) if match else None

            if not extract_date_value:
                Logger.log_error(f"Дата не найдена в названии файла: {self.filename}.")
                self.file_label.setText("Дата не найдена в названии файла.")
                return
            
            # Инициализируем TableEditor для выбранного файла
            self.editor = TableEditor(self.filename)

            # Добавляем столбцы, если их нет
            ADD_COLUMN_1 = "АВЗ (Целевая версия установлена, базы обновлены, политики применены.) СТАТУС НА МОМЕНТ ПРОВЕРКИ"
            ADD_COLUMN_2 = "Некорпоративные браузеры"
            self.editor.add_column_if_missing(ADD_COLUMN_1)
            self.editor.add_column_if_missing(ADD_COLUMN_2)
            self.editor.default_edit()  # Применяем нужное форматирование

        except Exception as e:
            Logger.log_error(f"Ошибка чтения файла: {str(e)}")
            self.file_label.setText(f"Ошибка: {str(e)}")