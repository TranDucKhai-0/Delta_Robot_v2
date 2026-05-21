import pygame
import socket
import math

# ================= CẤU HÌNH MẠNG =================
UDP_IP = "192.168.4.1" 
UDP_PORT = 1234      
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False) 

# ================= CẤU HÌNH VÙNG HOẠT ĐỘNG =================
R_MAX = 137.0      
Z_MIN = -350.0     
Z_MAX = -280.0     

# ================= CẤU HÌNH GIAO DIỆN =================
WIDTH, HEIGHT = 1000, 700  
PANEL_WIDTH = 400
CENTER = (PANEL_WIDTH + (WIDTH - PANEL_WIDTH) // 2, HEIGHT // 2) 
DRAW_RADIUS = 260

# Bảng Màu
BG_COLOR = (24, 24, 36)
PANEL_COLOR = (35, 35, 50)
TEXT_COLOR = (220, 220, 220)
HIGHLIGHT_COLOR = (0, 255, 150)
WARNING_COLOR = (255, 80, 80)
PICK_COLOR = (255, 165, 0)
PLACE_COLOR = (0, 255, 255)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Bảng Điều Khiển Delta Robot - Pro Version")
clock = pygame.time.Clock()

try:
    font_sm = pygame.font.SysFont("tahoma, segoeui", 16)
    font = pygame.font.SysFont("tahoma, segoeui", 20)
    font_large = pygame.font.SysFont("tahoma, segoeui", 24, bold=True)
except:
    font_sm = pygame.font.Font(pygame.font.get_default_font(), 16)
    font = pygame.font.Font(pygame.font.get_default_font(), 20) 
    font_large = pygame.font.Font(pygame.font.get_default_font(), 24)

# ================= CLASS TEXTBOX =================
class TextBox:
    def __init__(self, x, y, w, h, text=''):
        self.rect = pygame.Rect(x, y, w, h)
        self.color_inactive = (80, 80, 100)
        self.color_active = HIGHLIGHT_COLOR
        self.color = self.color_inactive
        self.text = text
        self.txt_surface = font.render(text, True, TEXT_COLOR)
        self.active = False

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            if self.rect.collidepoint(event.pos):
                self.active = True
            else:
                self.active = False
            self.color = self.color_active if self.active else self.color_inactive
            
        if event.type == pygame.KEYDOWN:
            if self.active:
                if event.key == pygame.K_BACKSPACE:
                    self.text = self.text[:-1]
                # Thêm K_ESCAPE vào danh sách bỏ qua để không in ra ký tự lạ
                elif event.key not in [pygame.K_RETURN, pygame.K_KP_ENTER, pygame.K_TAB, pygame.K_ESCAPE]:
                    if event.unicode in "0123456789- .":
                        self.text += event.unicode
                self.txt_surface = font.render(self.text, True, TEXT_COLOR)

    def draw(self, screen):
        pygame.draw.rect(screen, (20, 20, 30), self.rect)
        pygame.draw.rect(screen, self.color, self.rect, 2)
        screen.blit(self.txt_surface, (self.rect.x + 10, self.rect.y + 8))
        if self.active and pygame.time.get_ticks() % 1000 < 500:
            cursor_x = self.rect.x + 10 + self.txt_surface.get_width()
            pygame.draw.line(screen, HIGHLIGHT_COLOR, (cursor_x, self.rect.y + 8), (cursor_x, self.rect.y + 32), 2)

pick_input = TextBox(20, 430, 360, 40, "0.0 0.0 -300.0")
place_input = TextBox(20, 520, 360, 40, "0.0 0.0 -300.0")

# ================= TRẠNG THÁI BAN ĐẦU =================
mode = 0           
z_step = 5.0       

robot_x, robot_y, robot_z = 0.0, 0.0, -300.0   
pick_x, pick_y, pick_z = 0.0, 0.0, -300.0
place_x, place_y, place_z = 0.0, 0.0, -300.0

error_msg = ""
error_color = TEXT_COLOR

def map_mouse_to_robot(mx, my):
    dx = mx - CENTER[0]
    dy = my - CENTER[1]
    rx = (dx / DRAW_RADIUS) * R_MAX
    ry = -(dy / DRAW_RADIUS) * R_MAX
    
    distance = math.sqrt(rx**2 + ry**2)
    if distance > R_MAX:
        rx = (rx / distance) * R_MAX
        ry = (ry / distance) * R_MAX
    return rx, ry

def parse_xyz(text):
    parts = text.strip().split()
    if len(parts) == 3:
        return float(parts[0]), float(parts[1]), float(parts[2])
    raise ValueError

running = True
while running:
    # --- 1. XỬ LÝ SỰ KIỆN ---
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
            
        # Kiểm tra xem có ô nhập liệu nào đang được trỏ chuột (active) không
        is_typing = (mode == 3) and (pick_input.active or place_input.active)

        if event.type == pygame.KEYDOWN:
            if is_typing:
                # --- CHẾ ĐỘ ĐANG GÕ TEXT (Khóa đổi Mode) ---
                if event.key == pygame.K_ESCAPE:
                    # Bấm ESC để thoát trạng thái gõ text
                    pick_input.active = False
                    place_input.active = False
                    pick_input.color = pick_input.color_inactive
                    place_input.color = place_input.color_inactive
                elif event.key == pygame.K_TAB:
                    # Đổi ô nhập bằng phím TAB
                    if pick_input.active:
                        pick_input.active = False
                        pick_input.color = pick_input.color_inactive
                        place_input.active = True
                        place_input.color = place_input.color_active
                    else:
                        place_input.active = False
                        place_input.color = place_input.color_inactive
                        pick_input.active = True
                        pick_input.color = pick_input.color_active
                elif event.key in [pygame.K_RETURN, pygame.K_KP_ENTER]:
                    try:
                        px, py, pz = parse_xyz(pick_input.text)
                        lx, ly, lz = parse_xyz(place_input.text)
                        dist_pick = math.sqrt(px**2 + py**2)
                        dist_place = math.sqrt(lx**2 + ly**2)
                        
                        if dist_pick <= R_MAX and dist_place <= R_MAX and Z_MIN <= pz <= Z_MAX and Z_MIN <= lz <= Z_MAX:
                            pick_x, pick_y, pick_z = px, py, pz
                            place_x, place_y, place_z = lx, ly, lz
                            error_msg = "Cập nhật lệnh GẮP & THẢ thành công!"
                            error_color = HIGHLIGHT_COLOR
                        else:
                            error_msg = "Lỗi: Tọa độ vượt ngoài không gian làm việc!"
                            error_color = WARNING_COLOR
                    except ValueError:
                        error_msg = "Lỗi định dạng: Cần nhập đủ 'X Y Z' vào mỗi ô!"
                        error_color = WARNING_COLOR
            else:
                # --- CHẾ ĐỘ RẢNH TAY (Cho phép đổi Mode) ---
                if event.key in [pygame.K_0, pygame.K_KP0]: mode, error_msg = 0, ""
                elif event.key in [pygame.K_1, pygame.K_KP1]: mode, error_msg = 1, ""
                elif event.key in [pygame.K_2, pygame.K_KP2]: mode, error_msg = 2, ""
                elif event.key in [pygame.K_3, pygame.K_KP3]: mode, error_msg = 3, ""
                    
        elif event.type == pygame.MOUSEWHEEL:
            if mode in [2, 3]:
                if mode == 2:
                    robot_z += event.y * z_step
                    if robot_z < Z_MIN: robot_z = Z_MIN
                    if robot_z > Z_MAX: robot_z = Z_MAX

        # Đẩy sự kiện chuột/phím vào Class TextBox tự xử lý thêm ký tự
        if mode == 3:
            pick_input.handle_event(event)
            place_input.handle_event(event)

    # --- 2. CẬP NHẬT TỌA ĐỘ CHUỘT (Chỉ ở Mode 2) ---
    if mode == 2:
        mx, my = pygame.mouse.get_pos()
        if mx > PANEL_WIDTH:
            robot_x, robot_y = map_mouse_to_robot(mx, my)

    # --- 3. GỬI DỮ LIỆU XUỐNG ESP32 ---
    if mode in [0, 1]:
        msg_send = f"{mode}"
    elif mode == 2:
        msg_send = f"2,{robot_x:.5f},{robot_y:.5f},{robot_z:.5f}"
    elif mode == 3:
        msg_send = f"3,{pick_x:.5f},{pick_y:.5f},{pick_z:.5f},{place_x:.5f},{place_y:.5f},{place_z:.5f}"
        
    try:
        sock.sendto(msg_send.encode('utf-8'), (UDP_IP, UDP_PORT))
    except BlockingIOError:
        pass 

    # --- 4. VẼ GIAO DIỆN LÊN MÀN HÌNH ---
    screen.fill(BG_COLOR) 
    
    # 4.1 Vẽ Bảng Điều Khiển
    pygame.draw.rect(screen, PANEL_COLOR, (0, 0, PANEL_WIDTH, HEIGHT))
    pygame.draw.line(screen, HIGHLIGHT_COLOR, (PANEL_WIDTH, 0), (PANEL_WIDTH, HEIGHT), 2)
    
    screen.blit(font_large.render("BẢNG ĐIỀU KHIỂN ROBOT", True, TEXT_COLOR), (20, 20))
    
    modes_str = ["0. BACK HOME", "1. AUTO MODE", "2. MANUAL MODE", "3. PICK & PLACE"]
    for i, m_str in enumerate(modes_str):
        c = HIGHLIGHT_COLOR if mode == i else (120, 120, 120)
        screen.blit(font.render(m_str, True, c), (20, 80 + i * 35))
        
    pygame.draw.line(screen, (80, 80, 80), (20, 230), (PANEL_WIDTH - 20, 230), 1)

    screen.blit(font.render("TRẠNG THÁI TRUYỀN [UDP]:", True, (150, 150, 255)), (20, 250))
    if mode == 0: stat = "Chờ lệnh (Giữ Home)"
    elif mode == 1: stat = "Tự động sinh quỹ đạo"
    elif mode == 2: stat = f"X: {robot_x:.1f}  Y: {robot_y:.1f}  Z: {robot_z:.1f}"
    else: stat = "Hoạt động theo tọa độ đã chốt"
    screen.blit(font_large.render(stat, True, HIGHLIGHT_COLOR), (20, 280))
    
    # Khu vực thao tác Mode 3
    if mode == 3:
        screen.blit(font.render("--- THIẾT LẬP PICK & PLACE ---", True, PICK_COLOR), (20, 390))
        
        screen.blit(font_sm.render("Tọa độ GẮP (Pick): X Y Z", True, TEXT_COLOR), (20, 410))
        pick_input.draw(screen)
        
        screen.blit(font_sm.render("Tọa độ THẢ (Place): X Y Z", True, TEXT_COLOR), (20, 500))
        place_input.draw(screen)
        
        screen.blit(font_sm.render("Mẹo: Phím TAB để đổi ô | Bấm ESC để thoát gõ", True, (180, 180, 180)), (20, 580))
        screen.blit(font.render("Bấm [ENTER] để chốt gửi lệnh!", True, HIGHLIGHT_COLOR), (20, 600))
        
        if error_msg:
            screen.blit(font.render(error_msg, True, error_color), (20, 640))
            
    elif mode == 2:
        screen.blit(font.render("--- HƯỚNG DẪN MANUAL ---", True, TEXT_COLOR), (20, 390))
        screen.blit(font_sm.render("- Di chuyển chuột sang khung phải để đổi X, Y", True, (180, 180, 180)), (20, 430))
        screen.blit(font_sm.render("- Lăn chuột giữa (Scroll) để thay đổi độ cao Z", True, (180, 180, 180)), (20, 460))
        
    # 4.2 Vẽ Không Gian Làm Việc (Visualizer)
    grid_spacing = 40
    for i in range(-DRAW_RADIUS, DRAW_RADIUS, grid_spacing):
        pygame.draw.line(screen, (40, 40, 50), (CENTER[0] + i, CENTER[1] - DRAW_RADIUS), (CENTER[0] + i, CENTER[1] + DRAW_RADIUS), 1)
        pygame.draw.line(screen, (40, 40, 50), (CENTER[0] - DRAW_RADIUS, CENTER[1] + i), (CENTER[0] + DRAW_RADIUS, CENTER[1] + i), 1)

    pygame.draw.circle(screen, (80, 80, 100), CENTER, DRAW_RADIUS, 2)
    pygame.draw.line(screen, (100, 100, 120), (CENTER[0] - DRAW_RADIUS, CENTER[1]), (CENTER[0] + DRAW_RADIUS, CENTER[1]), 2)
    pygame.draw.line(screen, (100, 100, 120), (CENTER[0], CENTER[1] - DRAW_RADIUS), (CENTER[0], CENTER[1] + DRAW_RADIUS), 2)
    
    screen.blit(font_sm.render("+X", True, (100, 100, 120)), (CENTER[0] + DRAW_RADIUS + 5, CENTER[1] - 10))
    screen.blit(font_sm.render("+Y", True, (100, 100, 120)), (CENTER[0] - 10, CENTER[1] - DRAW_RADIUS - 20))

    if mode == 2:
        draw_x = int(CENTER[0] + (robot_x / R_MAX) * DRAW_RADIUS)
        draw_y = int(CENTER[1] - (robot_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, HIGHLIGHT_COLOR, (draw_x, draw_y), 8)
        pygame.draw.circle(screen, HIGHLIGHT_COLOR, (draw_x, draw_y), 15, 1) 
        
    elif mode == 3:
        dx_pick = int(CENTER[0] + (pick_x / R_MAX) * DRAW_RADIUS)
        dy_pick = int(CENTER[1] - (pick_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, PICK_COLOR, (dx_pick, dy_pick), 8)
        screen.blit(font_sm.render("PICK", True, PICK_COLOR), (dx_pick + 10, dy_pick - 10))
        
        dx_place = int(CENTER[0] + (place_x / R_MAX) * DRAW_RADIUS)
        dy_place = int(CENTER[1] - (place_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, PLACE_COLOR, (dx_place, dy_place), 8)
        screen.blit(font_sm.render("PLACE", True, PLACE_COLOR), (dx_place + 10, dy_place - 10))
        
        pygame.draw.line(screen, (150, 150, 150), (dx_pick, dy_pick), (dx_place, dy_place), 2)

    pygame.display.flip()
    clock.tick(60)

pygame.quit()