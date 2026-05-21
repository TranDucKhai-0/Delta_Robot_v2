import pygame
import socket
import math

# ==============================================================================
# CẤU HÌNH MẠNG & HỆ THỐNG
# ==============================================================================
UDP_IP = "192.168.4.1"
UDP_PORT = 1234
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False)

# ==============================================================================
# CẤU HÌNH ROBOT & GIAO DIỆN
# ==============================================================================
R_MAX = 137.0
Z_MIN = -350.0
Z_MAX = -280.0

# cấu hình độ rọng cửa sổ
WIDTH, HEIGHT = 900, 800
CENTER = (WIDTH // 2, HEIGHT // 2 - 100) # Đẩy tâm lên trên một chút
DRAW_RADIUS = int((min(WIDTH, HEIGHT) // 2) * 0.65)

# Bảng màu (Color Palette)
COLOR_BG = (25, 27, 33)
COLOR_PANEL = (35, 38, 46)
COLOR_TEXT = (220, 225, 235)
COLOR_PICK = (255, 150, 50)     # Cam
COLOR_PLACE = (50, 200, 255)    # Xanh dương
COLOR_MANUAL = (50, 255, 100)   # Xanh lá
COLOR_ERROR = (255, 80, 80)
COLOR_ACTIVE_BOX = (200, 200, 50)

# ==============================================================================
# KHỞI TẠO PYGAME
# ==============================================================================
pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Terminal: Control Board Delta Robot")
clock = pygame.time.Clock()

try:
    font_sm = pygame.font.SysFont("segoeui, tahoma", 16)
    font = pygame.font.SysFont("segoeui, tahoma", 20)
    font_large = pygame.font.SysFont("segoeui, tahoma", 26, bold=True)
except:
    font_sm = pygame.font.Font(pygame.font.get_default_font(), 16)
    font = pygame.font.Font(pygame.font.get_default_font(), 20)
    font_large = pygame.font.Font(pygame.font.get_default_font(), 26)

# ==============================================================================
# BIẾN TRẠNG THÁI (STATE)
# ==============================================================================
mode = 0
z_step = 5.0

robot_x, robot_y, robot_z = 0.0, 0.0, -300.0
pick_x, pick_y, pick_z = 0.0, 0.0, -281.0
place_x, place_y, place_z = 0.0, 0.0, -281.0

# Quản lý giao diện nhập liệu Mode 3
# "pick", "place", hoặc None
active_input = None 
input_text_pick = f"0.0 0.0 {Z_MAX-1}"
input_text_place = f"0.0 0.0 {Z_MAX-1}"
error_msg = ""
sys_msg = "Sẵn sàng."

msg_id = 0
send_count = 0
MAX_SEND_TIMES = 10

# Rects để bắt sự kiện click chuột vào ô nhập liệu
rect_box_pick = pygame.Rect(WIDTH // 2 - 320, HEIGHT - 120, 300, 50)
rect_box_place = pygame.Rect(WIDTH // 2 + 20, HEIGHT - 120, 300, 50)
 
# ==============================================================================
# CÁC HÀM XỬ LÝ LOGIC & DATA
# ==============================================================================
def trigger_new_command():
    """Lật ID (0 <-> 1) và khởi động quá trình bắn 10 gói UDP."""
    global msg_id, send_count
    msg_id = 1 - msg_id
    send_count = MAX_SEND_TIMES

def process_udp_transmission():
    """[HÀM ĐÓNG GÓI RIÊNG] Xử lý việc đóng gói chuỗi và gửi đi qua Socket UDP."""
    global send_count
    if send_count <= 0:
        return # Nếu không có lệnh mới thì im lặng, không xả rác ra mạng
        
    msg_send = ""
    if mode in [0, 1]:
        msg_send = f"{mode},{msg_id}"
    elif mode == 2:
        msg_send = f"2,{msg_id},{robot_x:.5f},{robot_y:.5f},{robot_z:.5f}"
    elif mode == 3:
        msg_send = f"3,{msg_id},{pick_x:.5f},{pick_y:.5f},{pick_z:.5f},{place_x:.5f},{place_y:.5f},{place_z:.5f}"

    if msg_send != "":
        try:
            sock.sendto(msg_send.encode('utf-8'), (UDP_IP, UDP_PORT))
            send_count -= 1
        except BlockingIOError:
            pass 

def map_mouse_to_robot(mx, my):
    """Quy đổi tọa độ pixel trên màn hình sang tọa độ thực của Robot."""
    dx = mx - CENTER[0]
    dy = my - CENTER[1]
    rx = (dx / DRAW_RADIUS) * R_MAX
    ry = -(dy / DRAW_RADIUS) * R_MAX
    
    distance = math.sqrt(rx**2 + ry**2)
    if distance > R_MAX:
        rx = (rx / distance) * R_MAX
        ry = (ry / distance) * R_MAX
    return rx, ry

def parse_point(text):
    """Hàm trích xuất 3 số thực (X, Y, Z) từ 1 ô nhập liệu."""
    parts = text.strip().split()
    if len(parts) != 3:
        return False, None, "Cần nhập đúng 3 số (X Y Z)."
    try:
        x, y, z = float(parts[0]), float(parts[1]), float(parts[2])
        if math.sqrt(x**2 + y**2) > R_MAX:
            return False, None, "Tọa độ X, Y vượt khỏi không gian làm việc!"
        if not (Z_MIN <= z <= Z_MAX):
            return False, None, f"Tọa độ Z phải từ {Z_MIN} đến {Z_MAX}!"
        return True, (x, y, z), ""
    except ValueError:
        return False, None, "Lỗi: Ký tự không hợp lệ, chỉ nhập số!"

# Gửi tín hiệu khởi tạo ngay khi mở app
trigger_new_command()

# ==============================================================================
# VÒNG LẶP CHÍNH (MAIN LOOP)
# ==============================================================================
running = True
while running:
    # --------------------------------------------------------------------------
    # XỬ LÝ SỰ KIỆN TỪ BÀN PHÍM VÀ CHUỘT
    # --------------------------------------------------------------------------
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
            
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1: # Click chuột trái
                if mode == 3:
                    if rect_box_pick.collidepoint(event.pos):
                        active_input = "pick"
                    elif rect_box_place.collidepoint(event.pos):
                        active_input = "place"
                    else:
                        active_input = None # Bấm ra ngoài thì bỏ focus

        elif event.type == pygame.MOUSEWHEEL:
            if mode == 2:
                robot_z += event.y * z_step
                robot_z = max(Z_MIN, min(robot_z, Z_MAX))
                trigger_new_command()

        elif event.type == pygame.KEYDOWN:
            # === ĐANG TRONG TRẠNG THÁI NHẬP LIỆU (MODE 3) ===
            if mode == 3 and active_input is not None:
                # Phím TAB: Chuyển đổi qua lại giữa 2 ô Pick và Place
                if event.key == pygame.K_TAB:
                    active_input = "place" if active_input == "pick" else "pick"
                    
                # Phím ENTER: Kích hoạt gửi UDP
                elif event.key in (pygame.K_RETURN, pygame.K_KP_ENTER):
                    # Parse cả 2 ô
                    ok_pick, p_pick, err_pick = parse_point(input_text_pick)
                    ok_place, p_place, err_place = parse_point(input_text_place)
                    
                    if not ok_pick:
                        error_msg = f"Lỗi ô GẮP: {err_pick}"
                    elif not ok_place:
                        error_msg = f"Lỗi ô THẢ: {err_place}"
                    else:
                        # Hợp lệ tất cả -> Lưu biến và kích hoạt UDP
                        pick_x, pick_y, pick_z = p_pick
                        place_x, place_y, place_z = p_place
                        error_msg = ""
                        sys_msg = ">> Đã gửi tọa độ Gắp/Thả thành công!"
                        active_input = None # Thoát chế độ gõ
                        trigger_new_command()
                        # NOTE: Cố ý KHÔNG reset chuỗi input_text_pick/place để giữ nguyên số.
                        
                # Phím ESC: Thoát chế độ gõ
                elif event.key == pygame.K_ESCAPE:
                    active_input = None
                    error_msg = ""
                    sys_msg = "Đã hủy nhập liệu."
                    
                # Xử lý Backspace (Xóa ký tự)
                elif event.key == pygame.K_BACKSPACE:
                    if active_input == "pick":
                        input_text_pick = input_text_pick[:-1]
                    else:
                        input_text_place = input_text_place[:-1]
                        
                # Xử lý gõ phím số và dấu
                else:
                    if event.unicode in "0123456789- .":
                        if active_input == "pick":
                            input_text_pick += event.unicode
                        else:
                            input_text_place += event.unicode
                        error_msg = ""
                        
            # === CÁC PHÍM TẮT ĐỔI MODE CHUNG ===
            else:
                if event.key in (pygame.K_0, pygame.K_KP0):
                    mode, active_input, error_msg = 0, None, ""
                    trigger_new_command()
                elif event.key in (pygame.K_1, pygame.K_KP1):
                    mode, active_input, error_msg = 1, None, ""
                    trigger_new_command()
                elif event.key in (pygame.K_2, pygame.K_KP2):
                    mode, active_input, error_msg = 2, None, ""
                    trigger_new_command()
                elif event.key in (pygame.K_3, pygame.K_KP3):
                    mode = 3
                    error_msg = ""
                    # Mặc định nhảy thẳng vào ô Pick cho tiện
                    active_input = "pick" 
                    # Tự động gán/hiển thị lại tọa độ Home (nếu muốn nó luôn reset khi vào mode 3)
                    # Nếu Đại Ca muốn vào mode 3 nó vẫn giữ số cũ của lần vào mode 3 trước đó,
                    # thì comment 3 dòng dưới này lại nhé.
                    home_z = Z_MAX - 1.0
                    pick_x = pick_y = place_x = place_y = 0.0
                    pick_z = place_z = home_z
                    input_text_pick = f"0.0 0.0 {home_z}"
                    input_text_place = f"0.0 0.0 {home_z}"
                    sys_msg = "Mode 3: Tự động tải tọa độ Home. Có thể gõ để sửa."
                    trigger_new_command()
                    
                # Bấm Enter ở ngoài thì tự nhảy vào Mode 3 và focus ô Pick
                elif event.key in (pygame.K_RETURN, pygame.K_KP_ENTER):
                    if mode != 3: 
                        mode = 3
                    active_input = "pick"

    # -- Cập nhật Manual Mode 2 --
    if mode == 2:
        mx, my = pygame.mouse.get_pos()
        # Chống lỗi: Chỉ cập nhật khi chuột nằm trong vùng vẽ workspace (nửa trên)
        if my < HEIGHT - 200: 
            new_x, new_y = map_mouse_to_robot(mx, my)
            if new_x != robot_x or new_y != robot_y:
                robot_x, robot_y = new_x, new_y
                trigger_new_command()

    # --------------------------------------------------------------------------
    # GỌI HÀM ĐÓNG GÓI VÀ GỬI UDP
    # --------------------------------------------------------------------------
    process_udp_transmission()

    # --------------------------------------------------------------------------
    # VẼ GIAO DIỆN (UI RENDER)
    # --------------------------------------------------------------------------
    screen.fill(COLOR_BG)
    
    # === VẼ WORKSPACE (Không gian làm việc) ===
    # Lưới grid nhạt
    for i in range(-5, 6):
        pygame.draw.line(screen, (40, 42, 50), (CENTER[0] + i*(DRAW_RADIUS//5), CENTER[1] - DRAW_RADIUS), (CENTER[0] + i*(DRAW_RADIUS//5), CENTER[1] + DRAW_RADIUS))
        pygame.draw.line(screen, (40, 42, 50), (CENTER[0] - DRAW_RADIUS, CENTER[1] + i*(DRAW_RADIUS//5)), (CENTER[0] + DRAW_RADIUS, CENTER[1] + i*(DRAW_RADIUS//5)))

    pygame.draw.circle(screen, (80, 85, 95), CENTER, DRAW_RADIUS, 2)
    pygame.draw.circle(screen, (255, 255, 255), CENTER, 3) # Tâm (0,0)

    if mode == 2:
        draw_x = int(CENTER[0] + (robot_x / R_MAX) * DRAW_RADIUS)
        draw_y = int(CENTER[1] - (robot_y / R_MAX) * DRAW_RADIUS)
        pygame.draw.circle(screen, COLOR_MANUAL, (draw_x, draw_y), 8)
        pygame.draw.circle(screen, (255, 255, 255), (draw_x, draw_y), 4) # Core
        
    elif mode == 3:
        dx_pick = int(CENTER[0] + (pick_x / R_MAX) * DRAW_RADIUS)
        dy_pick = int(CENTER[1] - (pick_y / R_MAX) * DRAW_RADIUS)
        dx_place = int(CENTER[0] + (place_x / R_MAX) * DRAW_RADIUS)
        dy_place = int(CENTER[1] - (place_y / R_MAX) * DRAW_RADIUS)
        
        # Đường nối
        pygame.draw.line(screen, (100, 100, 100), (dx_pick, dy_pick), (dx_place, dy_place), 2)
        
        # Điểm gắp/thả
        pygame.draw.circle(screen, COLOR_PICK, (dx_pick, dy_pick), 8)
        screen.blit(font_sm.render("PICK", True, COLOR_PICK), (dx_pick + 12, dy_pick - 10))
        
        pygame.draw.circle(screen, COLOR_PLACE, (dx_place, dy_place), 8)
        screen.blit(font_sm.render("PLACE", True, COLOR_PLACE), (dx_place + 12, dy_place - 10))

    # === VẼ BẢNG ĐIỀU KHIỂN BÊN DƯỚI (CONTROL PANEL) ===
    panel_rect = pygame.Rect(0, HEIGHT - 200, WIDTH, 200)
    pygame.draw.rect(screen, COLOR_PANEL, panel_rect)
    pygame.draw.line(screen, (70, 75, 85), (0, HEIGHT - 200), (WIDTH, HEIGHT - 200), 3)

    # Hiển thị tiêu đề Trạng Thái
    if mode == 3: mode_txt, m_color = "[3] PICK & PLACE", COLOR_PICK
    elif mode == 2: mode_txt, m_color = "[2] MANUAL (Mouse)", COLOR_MANUAL
    elif mode == 1: mode_txt, m_color = "[1] AUTO (Planner)", COLOR_TEXT
    else: mode_txt, m_color = "[0] BACK HOME", COLOR_ERROR

    screen.blit(font_large.render(f"MODE: {mode_txt}", True, m_color), (20, HEIGHT - 185))
    
    # Trạng thái mạng
    net_str = f"ID: {msg_id} | Đang đẩy gói tin... ({send_count}/10)" if send_count > 0 else f"ID: {msg_id} | Mạng: Idle"
    net_color = COLOR_MANUAL if send_count > 0 else (120, 125, 135)
    screen.blit(font.render(net_str, True, net_color), (WIDTH - 300, HEIGHT - 180))

    # === GIAO DIỆN CHUYÊN BIỆT TỪNG MODE ===
    if mode == 3:
        # Nhấp nháy con trỏ
        cursor = "|" if pygame.time.get_ticks() % 1000 < 500 else ""
        
        # VẼ Ô NHẬP LIỆU GẮP (PICK)
        pick_color = COLOR_ACTIVE_BOX if active_input == "pick" else (100, 105, 115)
        pygame.draw.rect(screen, (20, 22, 28), rect_box_pick, border_radius=8) # Nền ô
        pygame.draw.rect(screen, pick_color, rect_box_pick, 2, border_radius=8) # Viền ô
        
        screen.blit(font_sm.render("Pick (X Y Z)", True, COLOR_PICK), (rect_box_pick.x, rect_box_pick.y - 25))
        txt_p = input_text_pick + (cursor if active_input == "pick" else "")
        screen.blit(font_large.render(txt_p, True, COLOR_TEXT), (rect_box_pick.x + 15, rect_box_pick.y + 10))

        # VẼ Ô NHẬP LIỆU THẢ (PLACE)
        place_color = COLOR_ACTIVE_BOX if active_input == "place" else (100, 105, 115)
        pygame.draw.rect(screen, (20, 22, 28), rect_box_place, border_radius=8)
        pygame.draw.rect(screen, place_color, rect_box_place, 2, border_radius=8)
        
        screen.blit(font_sm.render("Place (X Y Z)", True, COLOR_PLACE), (rect_box_place.x, rect_box_place.y - 25))
        txt_l = input_text_place + (cursor if active_input == "place" else "")
        screen.blit(font_large.render(txt_l, True, COLOR_TEXT), (rect_box_place.x + 15, rect_box_place.y + 10))

        # HDSD Mode 3
        screen.blit(font_sm.render("Phím tắt: [Click chuột / TAB] để đổi ô | [ENTER] để chốt gửi | [ESC] để hủy", True, (150, 155, 165)), (WIDTH//2 - 250, HEIGHT - 35))

    elif mode == 2:
        screen.blit(font_large.render(f"Target -> X: {robot_x:.1f}   Y: {robot_y:.1f}   Z: {robot_z:.1f}", True, COLOR_TEXT), (20, HEIGHT - 120))
        screen.blit(font.render(">> Rê chuột ở vùng làm việc phía trên để di chuyển X/Y", True, COLOR_MANUAL), (20, HEIGHT - 80))
        screen.blit(font.render(">> Lăn chuột giữa để tăng/giảm trục Z", True, COLOR_MANUAL), (20, HEIGHT - 55))

    else:
        screen.blit(font_large.render("HỆ THỐNG ĐANG KHÓA VỊ TRÍ", True, (150, 150, 150)), (20, HEIGHT - 100))

    # Thông báo lỗi / hệ thống chung
    if error_msg:
        screen.blit(font.render(error_msg, True, COLOR_ERROR), (20, HEIGHT - 225))
    elif mode == 3 and sys_msg:
        screen.blit(font.render(sys_msg, True, COLOR_PICK), (20, HEIGHT - 225))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()