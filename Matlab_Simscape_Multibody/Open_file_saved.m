%% === PHẦN 1: TỰ ĐỘNG NHẬN DIỆN VÀ TRÍCH XUẤT DỮ LIỆU ===
clc; close all;

% 1. Kiểm tra xem dữ liệu đang ở đâu (Trong 'out' hay đã Load ra Workspace)
if exist('out', 'var') && isfield(out, 'Eo_x_Simscape')
    % Trường hợp 1: Vừa chạy xong Simulink (dữ liệu nằm trong out)
    disp('Đang lấy dữ liệu từ kết quả mô phỏng (out)...');
    raw_ik_x = out.Eo_x_IK; raw_ik_y = out.Eo_y_IK; raw_ik_z = out.Eo_z_IK;
    raw_sim_x = out.Eo_x_Simscape; raw_sim_y = out.Eo_y_Simscape; raw_sim_z = out.Eo_z_Simscape;
    raw_fk_x = out.Eo_x_FK; raw_fk_y = out.Eo_y_FK; raw_fk_z = out.Eo_z_FK;
    
elseif exist('Eo_x_Simscape', 'var')
    % Trường hợp 2: Đã Load từ file .mat (dữ liệu nằm rời bên ngoài)
    disp('Đang lấy dữ liệu từ file .mat đã load...');
    raw_ik_x = Eo_x_IK; raw_ik_y = Eo_y_IK; raw_ik_z = Eo_z_IK;
    raw_sim_x = Eo_x_Simscape; raw_sim_y = Eo_y_Simscape; raw_sim_z = Eo_z_Simscape;
    raw_fk_x = Eo_x_FK; raw_fk_y = Eo_y_FK; raw_fk_z = Eo_z_FK;
else
    % Trường hợp 3: Chưa có gì cả
    error('LỖI: Không tìm thấy dữ liệu! Hãy chạy lệnh: load(''Ten_File_Cua_May.mat'') trước khi chạy code này.');
end

% 2. Dùng hàm thông minh để tách dữ liệu (Hàm extract_data ở cuối bài)
[t, Sim_X] = extract_data(raw_sim_x);
[~, Sim_Y] = extract_data(raw_sim_y);
[~, Sim_Z] = extract_data(raw_sim_z);

[~, IK_X] = extract_data(raw_ik_x);
[~, IK_Y] = extract_data(raw_ik_y);
[~, IK_Z] = extract_data(raw_ik_z);

[~, FK_X] = extract_data(raw_fk_x);
[~, FK_Y] = extract_data(raw_fk_y);
[~, FK_Z] = extract_data(raw_fk_z);

%% ==========================================================
%% === PHẦN 2: VẼ ĐỒ THỊ (GIỮ NGUYÊN NHƯ CŨ) ===
%% ==========================================================

% --- FIGURE 1: X-axis Comparison (IK vs. Sim) ---
figure('Name', 'X-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_X, 'r--', 'LineWidth', 2); hold on; 
plot(t, Sim_X, 'b-', 'LineWidth', 1.5);        
grid on; xlabel('Time (s)'); 
ylabel('X-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('X-axis Position Response (IK vs Simscape)');

% --- FIGURE 2: Y-axis Comparison (IK vs. Sim) ---
figure('Name', 'Y-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_Y, 'r--', 'LineWidth', 2); hold on;
plot(t, Sim_Y, 'b-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Y-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Y-axis Position Response (IK vs Simscape)');

% --- FIGURE 3: Z-axis Comparison (IK vs. Sim) ---
figure('Name', 'Z-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_Z, 'r--', 'LineWidth', 2); hold on;
plot(t, Sim_Z, 'b-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Z-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Z-axis Position Response (IK vs Simscape)');

% --- FIGURE 4: Quỹ đạo 3D ---
figure('Name', 'Quỹ đạo 3D Thực tế', 'Color', 'w');
plot3(Sim_X, Sim_Y, Sim_Z, 'b-', 'LineWidth', 2); 
hold on; grid on; grid minor; axis equal;
plot3(Sim_X(1), Sim_Y(1), Sim_Z(1), 'go', 'MarkerSize', 10, 'MarkerFaceColor', 'g');
plot3(Sim_X(end), Sim_Y(end), Sim_Z(end), 'rs', 'MarkerSize', 10, 'MarkerFaceColor', 'r');
patch([-200 200 200 -200], [-200 -200 200 200], [0 0 0 0], 'k', 'FaceAlpha', 0.05, 'EdgeColor', 'none');
xlabel('X (mm)'); ylabel('Y (mm)'); zlabel('Z (mm)');
title('Actual trajectory in 3D space');
zlim([-450 0]); view([45, 30]); box on;


%% ==========================================================
%% === PHẦN 3: SO SÁNH FK (TÍNH TOÁN) vs SIMSCAPE (THỰC TẾ) ===
%% ==========================================================

% --- FIGURE 4: X-axis Comparison (FK vs. Sim) ---
figure('Name', 'X-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_X, 'k--', 'LineWidth', 2); hold on; 
plot(t, Sim_X, 'g-', 'LineWidth', 1.5);        
grid on; xlabel('Time (s)'); 
ylabel('X-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('X-axis Position Response (FK vs Simscape)');

% --- FIGURE 5: Y-axis Comparison (FK vs. Sim) ---
figure('Name', 'Y-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_Y, 'k--', 'LineWidth', 2); hold on;
plot(t, Sim_Y, 'g-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Y-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Y-axis Position Response (FK vs Simscape)');

% --- FIGURE 6: Z-axis Comparison (FK vs. Sim) ---
figure('Name', 'Z-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_Z, 'k--', 'LineWidth', 2); hold on;
plot(t, Sim_Z, 'g-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Z-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Z-axis Position Response (FK vs Simscape)');


%% ==========================================================
%% === HÀM PHỤ TRỢ QUAN TRỌNG (COPY ĐOẠN NÀY XUỐNG CUỐI CÙNG) ===
%% ==========================================================
function [time, data] = extract_data(raw_variable)
    % Hàm tự động xử lý Timeseries, Structure hoặc Matrix
    
    if isa(raw_variable, 'timeseries')
        % Nếu là Timeseries (Chuẩn Simulink)
        time = raw_variable.Time;
        data = squeeze(raw_variable.Data);
        
    elseif isstruct(raw_variable) && isfield(raw_variable, 'time')
        % Nếu là Struct (Do format save)
        time = raw_variable.time;
        data = squeeze(raw_variable.signals.values);
        
    elseif isnumeric(raw_variable)
        % Nếu là Ma trận (Array - Mất thông tin Time object)
        % Giả định cột 1 là Time, cột 2 là Data
        if size(raw_variable, 2) >= 2
            time = raw_variable(:, 1);
            data = squeeze(raw_variable(:, 2:end));
        else
            % Nếu chỉ có 1 cột data, tự tạo thời gian giả
            data = squeeze(raw_variable);
            time = 1:length(data); 
        end
    else
        error('Không nhận diện được định dạng dữ liệu!');
    end
end