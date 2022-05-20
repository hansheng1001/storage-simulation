from distutils import text_file
import matplotlib.pyplot as plt
from matplotlib import animation, image 
from matplotlib.widgets import Button, TextBox
import json
from matplotlib.backend_tools import ToolBase, ToolToggleBase
from PyQt5 import QtCore
from numpy import average
from mpl_toolkits.axes_grid1.inset_locator import inset_axes

task_data_file = "TaskAccount.json"
textBox_start = None
textBox_end = None
button_update = None
ax = None
data = []

def get_data(json_file):
    data = []
    with open(json_file, 'r') as file:
        lines = file.readlines()
        for line in lines:
            temp = json.loads(line)
            data.append(temp)
        return data

def get_delay_data(raw_data):
    delay_data = []
    for each in raw_data:
        time = each['Time']
        temp = []
        temp.append((time[1]-time[0]) / 1000000)
        temp.append((time[4] - time[2]) / 1000000)
        delay_data.append(temp)
    return delay_data

# def draw(data):
#     global ax
#     ax.set_title("task")
#     ax.set_xlabel("time(s)")
#     ax.set_ylabel("num")
#     colors = ['green', 'blue', 'red', 'black', 'orange', 'yellow']
#     cost_data = []
#     start = int(textBox_start.text)
#     end = int(textBox_end.text)
#     ax.set_ylim(start, end)
#     for i, each_data in enumerate(data):
#         #print(i, ":", each_data)
#         id = each_data['ID']
#         time = each_data['Time']
#         nodeId = each_data['NodeID']
#         if i >= start and i <= end:
#             if time[0] < time[1]:
#                 ax.fill_between([time[0] /1000, time[1]/1000], i, i+1)
#                 ax.text(time[0] /1000, i+0.2, 'sub:'+str(id), size=7)
#                 #ax.barh(i, time[1]/1000-time[0] /1000)
#             if time[2] < time[4]:
#                 ax.fill_between([time[2]/1000, time[4]/1000], i, i+1)
#                 #ax.text(time[4] /1000, i+0.2, 'sch:'+str(id), size=7)
#                 ax.text(time[2] /1000, i+0.2, 'sch', size=7)
#         #print(colors[i % len(colors)])
        # cost_data.append(time[4]/1000 - time[0] /1000)
        # ax.fill_between([i, i+1], 0, time[4]/1000 - time[0] /1000)
    #ax.plot([0, len(cost_data)], [average(cost_data), average(cost_data)])
    # ax.fill_between([0, len(cost_data)], 0, average(cost_data), color = 'yellow',alpha = 0.3)
def update(event):
    global ax, data
    if ax != None and len(data) > 0:
        draw(data)

def draw_2(ax, data):
    ax.set_title("task")
    ax.set_xlabel("time(ms)")
    ax.set_ylabel("num")
    colors = ['green', 'blue', 'red', 'black', 'orange', 'yellow']
    cost_data = []
    time_start = int(textBox_start.text)
    time_end = int(textBox_end.text)
    for i, each_data in enumerate(data):
        #print(i, ":", each_data)
        id = each_data['ID']
        time = each_data['Time']
        nodeId = each_data['NodeID']
        if time[0] >= time_start and time[4] <= time_end:
            print(each_data)

def draw(axs, data):
    level_submit = 0
    level_run = 0
    level_data_submit = []
    level_data_run = []
    for each in data:
        level_submit = max(each[0], level_submit)
        level_run = max(each[1], level_run)
    for i in range(int(level_submit)):
        level_data_submit.append(0)
    for i in range(int(level_run)):
        level_data_run.append(0)
    for each in data:
        level_data_submit[int(each[0] / 10)] += 1
        level_data_run[int(each[1] / 10)] += 1
    for i in range(len(level_data_submit)):
        if level_data_submit[i] != 0:
            axs[0].fill_between([i, i+1], 0,  level_data_submit[i])
    for i in range(len(level_data_run)):
        if level_data_run[i] != 0:
            axs[1].fill_between([i, i+1], 0,  level_data_run[i])
    axs[0].set_xlim(0, 200)
    #axs[0].set_ylim(0, 200)
    axs[0].set_ylim(0, max(level_data_submit))
    axs[0].set_title("submit delay")
    axs[0].set_xlabel("delay(ms)")
    axs[0].set_ylabel("num")
    axs[1].set_xlim(0, 200)
    #axs[1].set_ylim(0, 200)
    axs[0].set_ylim(0, max(level_data_run))
    axs[1].set_title("schedule delay")
    axs[0].set_xlabel("delay(ms)")
    axs[0].set_ylabel("num")

def task_run(event):
    global textBox_start, textBox_end, button_update, ax, data
    raw_data = get_data(task_data_file)
    delay_data = get_delay_data(raw_data)
    fig, axs = plt.subplots(2,1)
    fig.canvas.manager.window.showMaximized()
    fig.canvas.manager.window.setWindowTitle("TASK")
    # ax_start = fig.add_axes([0.1, 0.94, 0.05, 0.035])
    # ax_end = fig.add_axes([0.1, 0.90, 0.05, 0.035])
    # ax_update = fig.add_axes([0.1, 0.86, 0.05, 0.035])
    plt.subplots_adjust(top = 0.85, hspace = 0.3)
    plt.tight_layout()
    # textBox_start = TextBox(ax_start, 'start time:', '0')
    # textBox_end = TextBox(ax_end, 'end time:', '100')
    # button_update = Button(ax_update, 'update',color = 'lightgrey', hovercolor='grey')
    # button_update.on_clicked(update)
    #draw(data)
    draw(axs, delay_data)
    fig.show()

if __name__ == '__main__':
    task_run(None)