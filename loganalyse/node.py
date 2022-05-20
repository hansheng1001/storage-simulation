import matplotlib.pyplot as plt
from matplotlib import animation 
from matplotlib.widgets import Button, TextBox
import json
from matplotlib.backend_tools import ToolBase, ToolToggleBase
from PyQt5 import QtCore
from numpy import average, int64
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
import numpy as np
import threading
import time
import multiprocessing as mp
#import global_var as gb
node_data_file = "NodeAccount.json"
data = []
total_data = []
time_data = []
cpu_usage_data = []
disk_usage_data = []
memory_usage_data = []
net_send_data = []
net_recv_data = []
disk_read_data = []
disk_write_data = []
nodeNum = 0
diskNum = 0

anim1 = None
fig = None
axs = None
cpu_line = None
memory_line = None
net_line_send = None
net_line_recv = None
disk_read_line = None
disk_write_line = None
textBox_nodeNum = None
texBox_diskNum = None
button_play = None
button_pause = None
button_showAll = None
button_showTotal = None
cpu_text = None
memery_text = None
net_text = None
cpu_average_line = None
memory_average_line = None
net_average_line_send = None
net_average_line_recv = None
disk_read_average_line = None
disk_write_average_line = None

data_index = 0
play = False

def load_json(lines, process_index, sub_data):
    for line in lines:
        temp = json.loads(line)
        sub_data[process_index].append(temp)
    print(len(sub_data[process_index]))

# def get_raw_data(json_file):
#     global sub_data
#     time_start = time.time()
#     data = []
#     thread_list = []
#     thread_index = 0
#     with open(json_file, 'r') as file:
#         lines = file.readlines()
#         step = int(len(lines) / len(sub_data))
#         while thread_index < 4:
#             start = thread_index * step
#             end = start + step
#             if thread_index == 3:
#                 end = len(lines)
#             t = threading.Thread(target=load_json, args=(lines[start : end], thread_index))
#             t.start()
#             print("thread ", thread_index)
#             thread_list.append(t)
#             thread_index += 1
#         for m in thread_list:
#             m.join()
#         # for line in lines:#卡在这里
#         #     temp = json.loads(line)
#         #     data.append(temp)
#         for each in sub_data:
#             data += each
#         time_end = time.time()
#         print("run time: ", time_end-time_start)
#         return data

# def get_raw_data(json_file):
#     sub_data = mp.Manager().list()
#     for i in range(4):
#         sub_data.append([])
#     time_start = time.time()
#     data = []
#     process_index = 0
#     process_pool = mp.Pool(4)
#     with open(json_file, 'r') as file:
#         lines = file.readlines()
#         step = int(len(lines) / len(sub_data))
#         while process_index < 4:
#             start = process_index * step
#             end = start + step
#             if process_index == 3:
#                 end = len(lines)
#             process_pool.apply_async(load_json, args=(lines[start : end], process_index, sub_data))
#             print("process ", process_index)
#             process_index += 1

#         process_pool.close()
#         process_pool.join()
#         for each in sub_data:
#             data += each
#             #print(len(each))
#         time_end = time.time()
#         print("run time: ", time_end-time_start)
#         return data

def get_raw_data(json_file):
    time_start = time.time()
    data = []
    with open(json_file, 'r') as file:
        lines = file.readlines()
        for i,line in enumerate(lines):#卡在这里
            temp = json.loads(line)
            data.append(temp)
            proc_num = i / len(lines) * 100
            #gb.load_text.set_text("node data load {}%".format(proc_num) + "▋" * int(proc_num))
    time_end = time.time()
    print("run time: ", time_end-time_start)
    return data

def get_total_data(raw_data):
    data = []
    for line in raw_data:
        temp = line
        temp_dic = {"0":temp["0"][1], "1":temp["1"], 
                    "2":temp["2"], "3":[temp["3"][0],temp["3"][1],temp["3"][2],temp["3"][3]],
                    "4":[temp["4"][0], temp["4"][1], temp["4"][0] * temp["4"][2], temp["4"][1] * temp["4"][3]]}
        if len(data) == 0:
            data.append(temp_dic)
            continue
        each_data = data[len(data) - 1]
        if each_data['0'] == temp_dic['0']:
            each_data['1'] = np.array(each_data['1'], dtype=int64) + np.array(temp_dic['1'], dtype=int64)
            each_data['2'] = np.array(each_data['2'], dtype=int64) + np.array(temp_dic['2'], dtype=int64)
            each_data['3'] = np.array(each_data['3'], dtype=int64) + np.array(temp_dic['3'], dtype=int64)
            each_data['4'] = np.array(each_data['4'], dtype=int64) + np.array(temp_dic['4'], dtype=int64)
        else:
            data.append(temp_dic)
    return data

def draw(event):
    global anim1, data, time_data,raw_data
    global cpu_usage_data
    global disk_usage_data
    global memory_usage_data
    global net_send_data
    global net_recv_data
    global nodeNum, diskNum
    global data_index, play
    global cpu_text
    global disk_read_data, disk_write_data
    
    axs_init(axs[0][0], 'cpu usage')
    axs_init(axs[0][1],'memory usage')
    axs_init(axs[1][1], 'net usage')
    axs_init(axs[1][0], 'disk usage')
    nodeNum = int(textBox_nodeNum.text)
    data = []
    for each_data in raw_data:
        if each_data['0'][0] == nodeNum:
            data.append(each_data)
    time_data = []
    cpu_usage_data = []
    disk_usage_data = []
    memory_usage_data = []
    net_send_data = []
    net_recv_data = []
    disk_read_data = []
    disk_write_data = []
    diskNum = texBox_diskNum.text
    data_index = 0
    #data = get_data(node_data_file, nodeNum)
    if len(data) > 0: 
        if anim1 == None:
            play = True
            anim1=animation.FuncAnimation(fig, animate, 
                                        init_func=init,  
                                        frames=100000, 
                                        interval=200)
        else:
            play = True
            print(anim1)
            anim1.resume()
     
def axs_init(ax, title, xlim = (0, 200), ylim = (0,120), xlabel = 'time(ms)', ylabel = 'usage(%)'):
    ax.grid(True)
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel) 
    ax.set_xlim(xlim[0], xlim[1])
    ax.set_ylim(ylim[0], ylim[1])

def init(): 
    #初始化cpu曲线图
    cpu_line.set_data([], []) 
    cpu_average_line.set_data([], [])
    cpu_average_line.set(linestyle = '--', linewidth = 0.3, color='red')

    #初始化memory曲线图
    memory_line.set_data([], [])
    memory_average_line.set_data([], [])
    memory_average_line.set(linestyle = '--', linewidth = 0.3, color='red')

    #初始化net曲线图
    net_line_send.set_data([], [])
    net_line_send.set_label('send')
    net_average_line_send.set_data([], [])
    net_average_line_send.set_label('send_average')
    net_line_recv.set_data([], [])
    net_line_recv.set_label('recive')
    net_average_line_recv.set_data([], [])
    net_average_line_recv.set_label('revive_average')
    net_average_line_recv.set(linestyle = '--', linewidth = 0.3, color='g')
    net_average_line_send.set(linestyle = '--', linewidth = 0.3, color='lime')
    axs[1][1].legend(loc='upper left')

    #初始化disk曲线图
    disk_read_line.set_data([], [])
    disk_read_line.set_label('read')
    disk_read_average_line.set_data([], [])
    disk_read_average_line.set_label('read_average')
    disk_write_line.set_data([], [])
    disk_write_line.set_label('write')
    disk_write_average_line.set_data([], [])
    disk_write_average_line.set_label('write average')
    disk_write_average_line.set(linestyle = '--', linewidth = 0.3, color='g')
    disk_read_average_line.set(linestyle = '--', linewidth = 0.3, color='lime')
    axs[1][0].legend(loc='upper left')

    return cpu_line, memory_line, net_line_send, net_line_recv, disk_read_line, disk_write_line, \
            cpu_average_line,memory_average_line,net_average_line_recv,net_average_line_send,\
            disk_read_average_line, disk_write_average_line

def animate(i):
    global data_index, play
    if play and anim1 != None:
        if len(data) > data_index:
            each_data = data[data_index]
            data_index += 1
            time = each_data['0'][1] / 1000000

            # 绘制cpu利用率曲线
            cpu_usage = each_data['1'][0] * 100
            time_data.append(time)
            cpu_usage_data.append(cpu_usage)
            if time >= 200:
                axs[0][0].set_xlim(time-200, time)
                cpu_text.set_text("usedCoreNum:" +str(each_data['1'][2]) + '\ntotalCoreNum:' + str(each_data['1'][1]))
                cpu_text.set_position((time-50, 105))
            cpu_line.set_data(time_data, cpu_usage_data)
            cpu_text.set_text("usedCoreNum:" +str(each_data['1'][2]) + '\ntotalCoreNum:' + str(each_data['1'][1]))
            cpu_average_line.set_data([0, time], [average(cpu_usage_data), average(cpu_usage_data)])

            # 绘制memory曲线
            memory_usage = each_data['2'][0] * 100
            memory_usage_data.append(memory_usage)
            if time >= 200:
                axs[0][1].set_xlim(time-200, time)
                memery_text.set_text("usedMemory:" + str(each_data['2'][2]) + '\ntotalMemory:' + str(each_data['2'][1]))
                memery_text.set_position((time-75, 105))
            memory_line.set_data(time_data, memory_usage_data)
            memery_text.set_text("usedMemory:" + str(each_data['2'][2]) + '\ntotalMemory:' + str(each_data['2'][1]))
            memory_average_line.set_data([0, time], [average(memory_usage_data), average(memory_usage_data)])

            # 绘制net曲线
            net_send_bindwidth = each_data['4'][0]
            net_recv_bindwidth = each_data['4'][1]
            # net_send_usage = net_send_bindwidth * each_data['3'][2]
            # net_recv_usage = net_recv_bindwidth * each_data['3'][3]
            net_send_data.append(each_data['4'][2] * 100)
            net_recv_data.append(each_data['4'][3] * 100)
            
            if time >= 200:
                axs[1][1].set_xlim(time-200, time)
                net_text.set_position((time-85, 105))
            net_line_send.set_data(time_data, net_recv_data)
            net_line_recv.set_data(time_data, net_send_data)
            net_text.set_text("TotalSendBandWidth:"+ str(net_send_bindwidth) + "\nTotalReciveBandWidth:" + str(net_recv_bindwidth))
            net_average_line_send.set_data([0, time], [average(net_send_data), average(net_send_data)])
            net_average_line_recv.set_data([0, time], [average(net_recv_data), average(net_recv_data)])

            # 绘制disk曲线
            if diskNum == '':
                # disk_read_bindwidth = each_data['3'][1]
                # disk_write_bindwidth = each_data['3'][3]
                disk_read_data.append(each_data['3'][0] / each_data['3'][1] * 100)
                disk_write_data.append(each_data['3'][2] / each_data['3'][3] * 100)
            else:
                i = int(diskNum)+4
                if i >= len(each_data['3']) or i < 4:
                    i = len(each_data['3'])-1
                    texBox_diskNum.set_val(i-4)
                disk_read_data.append(each_data['3'][i][2] / each_data['3'][i][3] * 100)
                disk_write_data.append(each_data['3'][i][4] / each_data['3'][i][5] * 100)

            if time >= 200:
                axs[1][0].set_xlim(time-200, time)
            disk_read_line.set_data(time_data, disk_read_data)
            disk_write_line.set_data(time_data, disk_write_data)
            disk_read_average_line.set_data([0, time], [average(disk_read_data), average(disk_read_data)])
            disk_write_average_line.set_data([0, time], [average(disk_write_data), average(disk_write_data)])

    return cpu_line, memory_line, net_line_send, net_line_recv, disk_read_line, disk_write_line, \
            cpu_average_line,memory_average_line,net_average_line_recv,net_average_line_send,\
            disk_read_average_line, disk_write_average_line

def pause(event):
    global anim1, play
    print("pause:", anim1)
    if play and anim1 != None:
        anim1.pause()
        play = False
    elif not play and anim1 != None:
        anim1.resume()
        play = True

def show_all(event):
    init()
    global play, anim1, nodeNum,diskNum
    play = False
    time_data = []
    cpu_usage_data = []
    memory_usage_data = []
    net_send_data = []
    net_recv_data = []
    disk_read_data = []
    disk_write_data = []
    data = []
    nodeNum = int(textBox_nodeNum.text)
    for each_data in raw_data:
        if each_data['0'][0] == nodeNum:
            data.append(each_data)
    print("display all data get")
    for i in range(len(data)):
        each_data = data[i]
        time = each_data['0'][1] / 1000000
        cpu_usage = each_data['1'][0] * 100
        time_data.append(time)
        cpu_usage_data.append(cpu_usage)

        memory_usage = each_data['2'][0] * 100
        memory_usage_data.append(memory_usage)

        net_send_data.append(each_data['4'][2] * 100)
        net_recv_data.append(each_data['4'][3] * 100)

        diskNum = texBox_diskNum.text
        if diskNum == '':
                disk_read_data.append(each_data['3'][0] / each_data['3'][1] * 100)
                disk_write_data.append(each_data['3'][2] / each_data['3'][3] * 100)
        else:
            print(diskNum)
            i = int(diskNum)+3
            disk_read_data.append(each_data['3'][i][2] / each_data['3'][i][3] * 100)
            disk_write_data.append(each_data['3'][i][4] / each_data['3'][i][5] * 100)

    print("display all data load")
    axs_init(axs[0][0], 'cpu usage', (0, max(time_data)))
    axs_init(axs[0][1], 'memory usage', (0, max(time_data)))
    axs_init(axs[1][1], 'net usage', (0, max(time_data)))
    axs_init(axs[1][0], 'disk usage', (0, max(time_data)))
    cpu_line.set_data(time_data, cpu_usage_data)
    cpu_average_line.set_data([0, max(time_data)], [average(cpu_usage_data), average(cpu_usage_data)])
    cpu_text.set_text("")
    memory_line.set_data(time_data, memory_usage_data)
    memory_average_line.set_data([0, max(time_data)], [average(memory_usage_data), average(memory_usage_data)])
    memery_text.set_text("")
    net_line_send.set_data(time_data, net_recv_data)
    net_average_line_send.set_data([0, max(time_data)], [average(net_send_data), average(net_send_data)])
    net_line_recv.set_data(time_data, net_send_data)
    net_average_line_recv.set_data([0, max(time_data)], [average(net_recv_data), average(net_recv_data)])
    net_text.set_text("")
    disk_read_line.set_data(time_data, disk_read_data)
    disk_read_average_line.set_data([0, max(time_data)], [average(disk_read_data), average(disk_read_data)])
    disk_write_line.set_data(time_data, disk_write_data)
    disk_write_average_line.set_data([0, max(time_data)], [average(disk_write_data), average(disk_write_data)])

def show_total_all(event):
    init()
    global play, anim1, nodeNum,diskNum, total_data, raw_data
    play = False
    time_data = []
    cpu_usage_data = []
    memory_usage_data = []
    net_send_data = []
    net_recv_data = []
    disk_read_data = []
    disk_write_data = []
    if len(total_data) == 0:
        if len(raw_data) == 0:
            raw_data = get_raw_data(node_data_file)
        total_data = get_total_data(raw_data)
    
    for i in range(len(total_data)):
        each_data = total_data[i]
        time = each_data['0'] / 1000000
        time_data.append(time)
        cpu_usage = each_data['1'][2] / each_data['1'][1] * 100
        cpu_usage_data.append(cpu_usage)

        memory_usage = each_data['2'][2] / each_data['2'][1] * 100
        memory_usage_data.append(memory_usage)

        net_send_data.append(each_data['4'][2] / each_data['4'][0] * 100)
        net_recv_data.append(each_data['4'][3] / each_data['4'][1] * 100)

        disk_read_data.append(each_data['3'][0] / each_data['3'][1] * 100)
        disk_write_data.append(each_data['3'][2] / each_data['3'][3] * 100)

    print("display total data load")
    axs_init(axs[0][0], 'cpu usage', (0, max(time_data)))
    axs_init(axs[0][1], 'memory usage', (0, max(time_data)))
    axs_init(axs[1][1], 'net usage', (0, max(time_data)))
    axs_init(axs[1][0], 'disk usage', (0, max(time_data)))
    cpu_line.set_data(time_data, cpu_usage_data)
    cpu_average_line.set_data([0, max(time_data)], [average(cpu_usage_data), average(cpu_usage_data)])
    cpu_text.set_text("")
    memory_line.set_data(time_data, memory_usage_data)
    memory_average_line.set_data([0, max(time_data)], [average(memory_usage_data), average(memory_usage_data)])
    memery_text.set_text("")
    net_line_send.set_data(time_data, net_recv_data)
    net_average_line_send.set_data([0, max(time_data)], [average(net_send_data), average(net_send_data)])
    net_line_recv.set_data(time_data, net_send_data)
    net_average_line_recv.set_data([0, max(time_data)], [average(net_recv_data), average(net_recv_data)])
    net_text.set_text("")
    disk_read_line.set_data(time_data, disk_read_data)
    disk_read_average_line.set_data([0, max(time_data)], [average(disk_read_data), average(disk_read_data)])
    disk_write_line.set_data(time_data, disk_write_data)
    disk_write_average_line.set_data([0, max(time_data)], [average(disk_write_data), average(disk_write_data)])

def node_run(event):
    global fig, axs, data, anim1, total_data, raw_data
    global cpu_line, memory_line, net_line_send, net_line_recv,disk_read_line,disk_write_line,\
           cpu_average_line, memory_average_line, net_average_line_send, net_average_line_recv,\
           disk_read_average_line, disk_write_average_line
    global textBox_nodeNum, button_play, button_pause, button_showAll,texBox_diskNum, button_showTotal
    global cpu_text, memery_text,net_text
    raw_data = get_raw_data(node_data_file)
    total_data = get_total_data(raw_data)

    fig, axs = plt.subplots(2,2)
    plt.subplots_adjust(top = 0.85, hspace = 0.3)

    axs_init(axs[0][0], 'cpu usage')
    axs_init(axs[0][1],'memory usage')
    axs_init(axs[1][1], 'net usage')
    axs_init(axs[1][0], 'disk usage')
    #plt.tight_layout()
    fig.canvas.manager.window.showMaximized()
    fig.canvas.manager.window.setWindowTitle("NODE")
    # 文字标注
    cpu_text = axs[0][0].text(150, 105, "", size = 7)
    memery_text = axs[0][1].text(125, 105, "", size = 7)
    net_text = axs[1][1].text(115, 105, "", size = 7)
    
    # 控件
    ax_textBox = fig.add_axes([0.1, 0.94, 0.07, 0.035])
    ax_button_play = fig.add_axes([0.1, 0.9, 0.07, 0.035])
    ax_button_pause = fig.add_axes([0.18, 0.9, 0.07, 0.035])
    ax_button_showAll = fig.add_axes([0.26, 0.9, 0.07, 0.035]) 
    ax_button_showTotal = fig.add_axes([0.34, 0.9, 0.07, 0.035]) 
    ax_diskTextBox = inset_axes(axs[1][0],width = 0.5, height = 0.25,loc = 'upper right')
    textBox_nodeNum = TextBox(ax_textBox, 'node:', initial='0', textalignment='center')
    button_play= Button(ax_button_play, 'play', color = 'lightgrey', hovercolor='grey')
    button_pause= Button(ax_button_pause, 'pause/resume', color = 'lightgrey', hovercolor='grey')
    button_showAll = Button(ax_button_showAll, 'display_all', color = 'lightgrey', hovercolor='grey')
    button_showTotal = Button(ax_button_showTotal, 'display_total', color = 'lightgrey', hovercolor='grey')
    texBox_diskNum = TextBox(ax_diskTextBox, 'disk:', initial='', textalignment='center')
    button_play.on_clicked(draw)
    button_pause.on_clicked(pause)
    button_showAll.on_clicked(show_all)
    button_showTotal.on_clicked(show_total_all)

    # 曲线
    cpu_line, = axs[0][0].plot([], []) 
    memory_line, = axs[0][1].plot([], [])
    net_line_send, = axs[1][1].plot([], [])
    net_line_recv, = axs[1][1].plot([], [])
    disk_read_line, = axs[1][0].plot([], [])
    disk_write_line, = axs[1][0].plot([], [])
    cpu_average_line, = axs[0][0].plot([], [])
    memory_average_line, = axs[0][1].plot([], [])
    net_average_line_send, = axs[1][1].plot([], [])
    net_average_line_recv, = axs[1][1].plot([], [])
    disk_read_average_line, = axs[1][0].plot([], [])
    disk_write_average_line, = axs[1][0].plot([], [])
    
    fig.show()
    anim1 = None

if __name__ == "__main__":
    raw_data = get_raw_data(node_data_file)