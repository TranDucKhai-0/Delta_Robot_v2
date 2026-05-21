% Mở hộp thoại chọn file để mày chọn đúng file cần load
[file, path] = uigetfile('*.mat', 'Chọn file dữ liệu cũ');

if isequal(file, 0)
    disp('Chưa chọn file nào cả.');
else
    load(fullfile(path, file));
    disp('Đã load lại dữ liệu!');
    
    % Kiểm tra xem biến đã vào chưa
    whos Error_IK Error_FK Orbit
end