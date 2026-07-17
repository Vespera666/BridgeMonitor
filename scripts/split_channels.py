#!/usr/bin/env python3
"""
将 7 个传感器原始数据文件（GBK编码、Tab分隔）按通道拆分为独立 CSV。
每个通道 = 一个监测点，输出文件以监测点编号命名。

输出格式：UTF-8, 逗号分隔, 时间格式 yyyy-MM-dd HH:mm:ss
与 FileManager::readCsv() 完全兼容。
"""
import os
import glob
import re
from datetime import datetime


def parse_time(raw: str) -> str:
    """将 '2023/1/1 1:10' 格式化为 '2023-01-01 01:10:00'"""
    raw = raw.strip()
    try:
        dt = datetime.strptime(raw, "%Y/%m/%d %H:%M")
    except ValueError:
        try:
            dt = datetime.strptime(raw, "%Y/%m/%d %H:%M:%S")
        except ValueError:
            return raw
    return dt.strftime("%Y-%m-%d %H:%M:%S")


# ── 传感器类型定义 ──────────────────────────────────────────────
# 键 = 文件特征 (用文件大小区分)
# 值 = (类型中文名, 字段列表: [(后缀, 字段名, 单位), ...])
#      无A/B后缀的通道 => 后缀为空字符串

SENSOR_CONFIG = {
    # 文件大小 -> 配置
    1225088: {  # 索力
        "type": "索力",
        "fields": [("", "索力值", "kN")],
        "csv_header": "时间,索力值(kN)",
    },
    261623: {  # 挠度
        "type": "挠度",
        "fields": [("", "挠度", "mm")],
        "csv_header": "时间,挠度(mm)",
    },
    174788: {  # 振动
        "type": "振动",
        "fields": [("", "振动频率", "Hz")],
        "csv_header": "时间,振动频率(Hz)",
    },
    140286: {  # 支座位移
        "type": "支座位移",
        "fields": [("", "位移量", "mm")],
        "csv_header": "时间,位移量(mm)",
    },
    106946: {  # 伸缩缝
        "type": "伸缩缝",
        "fields": [("", "伸缩量", "mm")],
        "csv_header": "时间,伸缩量(mm)",
    },
    97527: {  # 风速风向 — A=风速, B=风向
        "type": "风速风向",
        "fields": [("-A", "风速", "m/s"), ("-B", "风向", "°")],
        "csv_header_a": "时间,风速(m/s)",
        "csv_header_b": "时间,风向(°)",
    },
    92369: {  # 温湿度 — A=温度, B=湿度
        "type": "温湿度",
        "fields": [("-A", "温度", "°C"), ("-B", "湿度", "%")],
        "csv_header_a": "时间,温度(°C)",
        "csv_header_b": "时间,湿度(%)",
    },
}


def split_file(filepath: str, output_dir: str) -> int:
    """拆分一个文件，返回生成的 CSV 数量"""
    size = os.path.getsize(filepath)
    config = SENSOR_CONFIG.get(size)
    if config is None:
        print(f"  ⚠ 未知文件大小 {size}，跳过: {os.path.basename(filepath)}")
        return 0

    # 读取 GBK 编码的 Tab 分隔文件
    with open(filepath, "r", encoding="gbk") as f:
        lines = f.readlines()

    if not lines:
        return 0

    header_line = lines[0].strip()
    if not header_line:
        return 0

    columns = header_line.split("\t")
    # 第0列是"时间"，其余列是各通道
    channels = [c.strip() for c in columns[1:]]

    # 解析数据行
    data_rows = []
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        time_str = parse_time(parts[0])
        values = [p.strip() for p in parts[1:]]
        data_rows.append((time_str, values))

    print(f"  {config['type']}: {len(channels)} 通道, {len(data_rows)} 行数据")

    count = 0
    for ci, ch_name in enumerate(channels):
        # 判断是否是 A/B 后缀通道（如 L7_WSD11_1-A）
        suffix = ""
        base_name = ch_name
        is_ab = False
        if ch_name.endswith("-A") or ch_name.endswith("-B"):
            is_ab = True
            # 保留后缀，因为监测点编号可能就带 -A/-B
            # 先检查一下：监测点列表里温湿度和风速风向的编号确实带 -A/-B

        # 确定 CSV 表头
        if is_ab:
            if ch_name.endswith("-A"):
                csv_header = config.get("csv_header_a", "时间,值")
            else:
                csv_header = config.get("csv_header_b", "时间,值")
        else:
            csv_header = config.get("csv_header", "时间,值")

        # 提取该通道的数据
        rows = []
        for time_str, values in data_rows:
            if ci < len(values):
                rows.append(f"{time_str},{values[ci]}")
            else:
                rows.append(f"{time_str},")

        # 写入 CSV
        filename = ch_name + ".csv"
        out_path = os.path.join(output_dir, filename)
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(csv_header + "\n")
            f.write("\n".join(rows) + "\n")

        count += 1

    return count


def main():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    build_dir = os.path.join(base_dir, "build")
    output_dir = os.path.join(base_dir, "data")

    os.makedirs(output_dir, exist_ok=True)

    xls_files = glob.glob(os.path.join(build_dir, "*.xls"))
    xls_files.sort(key=os.path.getsize, reverse=True)

    print(f"找到 {len(xls_files)} 个 .xls 数据文件")
    print(f"输出目录: {output_dir}")
    print("=" * 60)

    total = 0
    for fp in xls_files:
        basename = os.path.basename(fp)
        print(f"处理: {basename} ({os.path.getsize(fp):,} bytes)")
        n = split_file(fp, output_dir)
        total += n

    print("=" * 60)
    print(f"完成! 共生成 {total} 个 CSV 文件 → {output_dir}/")

    # 列出生成的统计
    csv_files = glob.glob(os.path.join(output_dir, "*.csv"))
    type_count = {}
    for cf in csv_files:
        name = os.path.basename(cf)
        # 从文件名推断类型
        parts = name.replace(".csv", "").split("_")
        if len(parts) >= 3:
            t = parts[1]  # ND, SL, ZD, etc.
        elif len(parts) >= 2:
            t = parts[0]
        else:
            t = "?"
        type_count[t] = type_count.get(t, 0) + 1

    print("\n按传感器类型统计:")
    type_labels = {
        "SL": "索力(索力值/kN)",
        "ND": "挠度(挠度/mm)",
        "ZD": "振动(振动频率/Hz)",
        "ZY": "支座位移(位移量/mm)",
        "LY": "伸缩缝(伸缩量/mm)",
        "FS": "风速风向(风速-A / 风向-B)",
        "WSD": "温湿度(温度-A / 湿度-B)",
    }
    for t, c in sorted(type_count.items()):
        label = type_labels.get(t, t)
        print(f"  {label}: {c} 个文件")


if __name__ == "__main__":
    main()
