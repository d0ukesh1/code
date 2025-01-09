from PyQt6.QtWidgets import QLabel, QVBoxLayout, QDialog
from PyQt6.QtCore import Qt, QTimer, QRect
from PyQt6.QtGui import QPixmap,QIcon
import sys, os

def resource_path(relative_path):
    """ Получает абсолютный путь к ресурсам, которые добавлены в .exe. """
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return relative_path

class SplashScreen(QDialog):
    """Класс для отображения заставки при запуске программы."""
    def __init__(self, duration=2000, parent=None):
        super().__init__(parent)
        
        # Настройка окна заставки
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setModal(True)
        icon_path = resource_path("ui/alrosa.ico")
        self.setWindowIcon(QIcon(icon_path))
        # Получаем размеры экрана
        screen_geometry = self.screen().availableGeometry()
        screen_width = screen_geometry.width()
        screen_height = screen_geometry.height()

        # Загрузка изображения и его масштабирование
        start_logo = resource_path("ui/alrosa_start.png")
        pixmap = QPixmap(start_logo)
        pixmap = pixmap.scaled(screen_width // 4, screen_height // 4, Qt.AspectRatioMode.KeepAspectRatio)

        # Логотип
        logo = QLabel()
        logo.setPixmap(pixmap)
        logo.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Компоновка
        layout = QVBoxLayout(self)
        layout.addWidget(logo)
        self.setLayout(layout)

        # Центрируем окно заставки на экране
        self.setGeometry(QRect((screen_width - pixmap.width()) // 2, (screen_height - pixmap.height()) // 2, pixmap.width(), pixmap.height()))

        # Закрытие заставки по таймеру
        QTimer.singleShot(duration, self.accept)