from matplotlib.widgets import Button, TextBox
import matplotlib.pyplot as plt
import node
import task
#import global_var as gb
if __name__ == '__main__':
    fig = plt.figure()
    fig.canvas.manager.window.setWindowTitle("SIMULATE")
    axes_button_node = fig.add_axes([0.5-0.1, 0.5+0.01, 0.2, 0.1])
    axes_button_task = fig.add_axes([0.5-0.1, 0.5-0.1, 0.2, 0.1])
    #gb.load_text = fig.text(0.4, 0.35, "▋")
    button_node = Button(axes_button_node, 'node', color = 'white', 
                              hovercolor = 'lightgrey')#节点
    button_task = Button(axes_button_task, 'task', color = 'white', 
                              hovercolor = 'lightgrey')#任务
    button_node.on_clicked(node.node_run)   
    button_task.on_clicked(task.task_run)
    plt.show()