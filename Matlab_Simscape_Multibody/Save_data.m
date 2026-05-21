%% === CẤU HÌNH TÊN FILE ===
fileName = ['Data_Full_Robot_Line_' '.mat'];

%% === TRÍCH XUẤT DỮ LIỆU TỪ BIẾN 'out' ===
if exist('out', 'var')
    % 1. Dữ liệu Quỹ đạo gốc (Orbit) - QUAN TRỌNG
    if isfield(out, 'Orbit')
        Orbit = out.Orbit;
    else
        warning('Không tìm thấy biến Orbit trong out!');
        Orbit = []; % Tạo rỗng để không lỗi save
    end

    % 2. Dữ liệu IK (Mong muốn)
    Eo_x_IK = out.Eo_x_IK;
    Eo_y_IK = out.Eo_y_IK;
    Eo_z_IK = out.Eo_z_IK;
    
    % 3. Dữ liệu Simscape (Thực tế)
    Eo_x_Simscape = out.Eo_x_Simscape;
    Eo_y_Simscape = out.Eo_y_Simscape;
    Eo_z_Simscape = out.Eo_z_Simscape;
    
    % 4. Dữ liệu FK (Tính toán kiểm chứng)
    Eo_x_FK = out.Eo_x_FK;
    Eo_y_FK = out.Eo_y_FK;
    Eo_z_FK = out.Eo_z_FK;
else
    error('Lỗi: Không tìm thấy biến "out". Hãy chạy mô phỏng trước!');
end

%% === LƯU TẤT CẢ VÀO FILE .MAT ===
% Liệt kê tất cả các biến muốn lưu
save(fileName, ...
    'Orbit', ...                          % <--- Đã thêm Quỹ đạo
    'Eo_x_IK', 'Eo_y_IK', 'Eo_z_IK', ...
    'Eo_x_Simscape', 'Eo_y_Simscape', 'Eo_z_Simscape', ...
    'Eo_x_FK', 'Eo_y_FK', 'Eo_z_FK');              % Lưu luôn sai số cho chắc cú

disp(['------------------------------------------------']);
disp(['Đã lưu Full Data (Quỹ đạo + Tọa độ + Sai số) vào file:']);
disp(fileName);
disp(['------------------------------------------------']);