#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining_bt;
    int waiting;
    int tat;
    int ct; 
} Process;

typedef struct {
    int process_id;
    int start_time;
    int end_time;
} GanttEntry;


int validate_inputs(Process processes[], int n, int quantum);
void input_processes(Process processes[], int n, int algo_choice);
void reset_processes(Process dest[], Process src[], int n);
void print_results(Process processes[], int n, float awt, float atat);
void display_gantt_chart(GanttEntry gantt[], int count);
void fcfs(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index);
void sjf_non_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index);
void sjf_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index);
void rr(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index, int quantum);
void priority_non_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index);
void priority_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index);

int validate_inputs(Process processes[], int n, int quantum) {
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival < 0 || processes[i].burst <= 0 || processes[i].priority < 0) {
            printf("Invalid input: Arrival time, burst time, and priority must be non-negative, and burst time must be positive.\n");
            return 0;
        }
    }
    if (quantum <= 0) {
        printf("Invalid input: Time quantum must be positive for Round Robin.\n");
        return 0;
    }
    return 1;
}

void input_processes(Process processes[], int n, int algo_choice) {
    for (int i = 0; i < n; i++) {
        processes[i].id = i + 1;
        printf("Process %d\n", i + 1);
        printf("Arrival Time: "); scanf("%d", &processes[i].arrival);
        printf("Burst Time: "); scanf("%d", &processes[i].burst);
        // Ask for priority if the algorithm is Priority-based
        if (algo_choice == 4) {
            printf("Priority (lower number = higher priority): "); scanf("%d", &processes[i].priority);
        } else {
            processes[i].priority = 0; 
        }
        processes[i].remaining_bt = processes[i].burst;
    }
}

void reset_processes(Process dest[], Process src[], int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
        dest[i].remaining_bt = dest[i].burst;
        dest[i].waiting = 0;
        dest[i].tat = 0;
        dest[i].ct = 0; 
    }
}

void print_results(Process processes[], int n, float awt, float atat) {
    printf("\nID\tArrival\tBurst\tPriority\tCT\tWaiting\tTAT\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t\t%d\t%d\t%d\n", 
               processes[i].id, processes[i].arrival,
               processes[i].burst, processes[i].priority,
               processes[i].ct, processes[i].waiting, processes[i].tat);
    }
    printf("\nAverage Waiting Time: %.2f\n", awt);
    printf("Average Turnaround Time: %.2f\n", atat);
}

void display_gantt_chart(GanttEntry gantt[], int count) {
    printf("\nGantt Chart:\n");
    for (int i = 0; i < count; i++) {
        printf("| P%d (%d-%d) ", gantt[i].process_id, 
              gantt[i].start_time, gantt[i].end_time);
    }
    printf("|\n");
    

    for (int i = 0; i < count; i++) {
        printf("\nProcessing P%d [", gantt[i].process_id);
        int duration = gantt[i].end_time - gantt[i].start_time;
        for (int j = 0; j < duration; j++) {
            printf("#");
            fflush(stdout);
            sleep(1);
        }
        printf("] %d-%d ", gantt[i].start_time, gantt[i].end_time);
    }
    printf("\n");
}

void fcfs(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index) {
    Process temp[n];
    memcpy(temp, processes, sizeof(temp));
    
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n-1; j++) {
            if (temp[j].arrival > temp[j+1].arrival) {
                Process swap = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = swap;
            }
        }
    }

    int current_time = 0;
    float total_wt = 0, total_tat = 0;
    
    for (int i = 0; i < n; i++) {
        if (current_time < temp[i].arrival)
            current_time = temp[i].arrival;
        
        
        gantt[*gantt_index].process_id = temp[i].id;
        gantt[*gantt_index].start_time = current_time;
        gantt[*gantt_index].end_time = current_time + temp[i].burst;
        (*gantt_index)++;
            
        temp[i].waiting = current_time - temp[i].arrival;
        current_time += temp[i].burst;
        temp[i].tat = temp[i].waiting + temp[i].burst;
        temp[i].ct = current_time;  
        
        total_wt += temp[i].waiting;
        total_tat += temp[i].tat;
    }
    
    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(temp, n, results[0], results[1]);
}

void sjf_non_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index) {
    Process temp[n];
    memcpy(temp, processes, sizeof(temp));
    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;

    while (completed < n) {
        int shortest = -1;
        int min_burst = 9999;
        
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= current_time && 
               temp[i].remaining_bt > 0 &&
               temp[i].burst < min_burst) {
                shortest = i;
                min_burst = temp[i].burst;
            }
        }
        
        if (shortest == -1) {
            current_time++;
            continue;
        }
        
         
        gantt[*gantt_index].process_id = temp[shortest].id;
        gantt[*gantt_index].start_time = current_time;
        gantt[*gantt_index].end_time = current_time + temp[shortest].burst;
        (*gantt_index)++;
        
        current_time += temp[shortest].burst;
        temp[shortest].remaining_bt = 0;
        temp[shortest].tat = current_time - temp[shortest].arrival;
        temp[shortest].waiting = temp[shortest].tat - temp[shortest].burst;
        temp[shortest].ct = current_time;  
        
        total_wt += temp[shortest].waiting;
        total_tat += temp[shortest].tat;
        completed++;
    }
    
    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(temp, n, results[0], results[1]);
}

void sjf_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index) {
    Process temp[n];
    memcpy(temp, processes, sizeof(temp));
    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;
    int current_process = -1;  
    int first_arrival = 9999; 

     
    for (int i = 0; i < n; i++) {
        if (temp[i].arrival < first_arrival) {
            first_arrival = temp[i].arrival;
        }
    }

  
    current_time = first_arrival;

    while (completed < n) {
        
        int new_arrival = 0;
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival == current_time) {
                new_arrival = 1;
                break;
            }
        }

        
        int shortest = -1;
        int min_remaining = 9999;
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= current_time && 
                temp[i].remaining_bt > 0 && 
                temp[i].remaining_bt < min_remaining) {
                shortest = i;
                min_remaining = temp[i].remaining_bt;
            } else if (temp[i].arrival <= current_time && 
                      temp[i].remaining_bt > 0 && 
                      temp[i].remaining_bt == min_remaining) {
            
                if (current_process != -1 && i == current_process) {
                    shortest = i;
                } else if (shortest == -1 || temp[i].id < temp[shortest].id) {
                    shortest = i;
                }
            }
        }

     
        if (shortest == -1) {
         
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
                current_process = -1;
            }
            current_time++;
            continue;
        }

     
        if (current_process != shortest || new_arrival) {
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
            }
            gantt[*gantt_index].process_id = temp[shortest].id;
            gantt[*gantt_index].start_time = current_time;
            current_process = shortest;
        }

        
        temp[shortest].remaining_bt--;
        current_time++;

         
        if (temp[shortest].remaining_bt == 0) {
            temp[shortest].tat = current_time - temp[shortest].arrival;
            temp[shortest].waiting = temp[shortest].tat - temp[shortest].burst;
            temp[shortest].ct = current_time; 
            total_wt += temp[shortest].waiting;
            total_tat += temp[shortest].tat;
            completed++;
            current_process = -1; 
            gantt[*gantt_index].end_time = current_time;
            (*gantt_index)++;
        }
    }

    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(temp, n, results[0], results[1]);
}

void rr(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index, int quantum) {
    Process temp[n];
    memcpy(temp, processes, sizeof(temp));
    int completed = 0, current_time = 0;
    int ready_queue[1000], front = 0, rear = -1; 
    float total_wt = 0, total_tat = 0;
    int current_process = -1;

 
    int first_arrival = 9999;
    for (int i = 0; i < n; i++) {
        if (temp[i].arrival < first_arrival) {
            first_arrival = temp[i].arrival;
        }
    }
    current_time = first_arrival;

     
    for (int i = 0; i < n; i++) {
        if (temp[i].arrival <= current_time && temp[i].remaining_bt > 0) {
            ready_queue[++rear] = i;
        }
    }

    printf("\nRound Robin Scheduling with Time Quantum: %d\n", quantum);
    while (completed < n) { 
        if (front > rear) { 
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
                current_process = -1;
            }
            current_time++; 
            for (int i = 0; i < n; i++) {
                if (temp[i].arrival == current_time && temp[i].remaining_bt > 0) {
                    ready_queue[++rear] = i;
                }
            }
            continue;
        }
 
        int idx = ready_queue[front++];
 
        if (current_process != idx) {
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
            }
            gantt[*gantt_index].process_id = temp[idx].id;
            gantt[*gantt_index].start_time = current_time;
            current_process = idx;
        }
 
        int exec_time = (temp[idx].remaining_bt > quantum) ? quantum : temp[idx].remaining_bt;
        for (int t = 0; t < exec_time; t++) {
            temp[idx].remaining_bt--;
            current_time++;
 
            for (int i = 0; i < n; i++) {
                if (temp[i].arrival == current_time && temp[i].remaining_bt > 0) {
                    ready_queue[++rear] = i;
                }
            }
  
            if (temp[idx].remaining_bt == 0) {
                temp[idx].tat = current_time - temp[idx].arrival;
                temp[idx].waiting = temp[idx].tat - temp[idx].burst;
                temp[idx].ct = current_time; // Completion time
                total_wt += temp[idx].waiting;
                total_tat += temp[idx].tat;
                completed++; 
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
                current_process = -1;
                break;  
            }
        }
 
        if (temp[idx].remaining_bt > 0) {
            ready_queue[++rear] = idx;
         
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
                current_process = -1;
            }
        }
    }

    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(temp, n, results[0], results[1]);
}

void priority_non_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index) {
    int scheduled = 0;
    int is_completed[n];
    for (int i = 0; i < n; i++) {
        is_completed[i] = 0;
    }

    int current_time = 0;
    float total_wt = 0, total_tat = 0;

    
    while (scheduled < n) {
        int idx = -1;
        int highest_priority = 1000000;  
 
        for (int i = 0; i < n; i++) {
            if (!is_completed[i] && processes[i].arrival <= current_time) {
                if (processes[i].priority < highest_priority) {
                    highest_priority = processes[i].priority;
                    idx = i;
                }
            }
        }
 
        if (idx == -1) {
            int min_arrival = 1000000;
            for (int i = 0; i < n; i++) {
                if (!is_completed[i] && processes[i].arrival < min_arrival) {
                    min_arrival = processes[i].arrival;
                    idx = i;
                }
            }
            current_time = processes[idx].arrival;
        }
 
        gantt[*gantt_index].process_id = processes[idx].id;
        gantt[*gantt_index].start_time = current_time;
        gantt[*gantt_index].end_time = current_time + processes[idx].burst;
        (*gantt_index)++;
 
        processes[idx].waiting = current_time - processes[idx].arrival;
        current_time += processes[idx].burst;
        processes[idx].tat = processes[idx].waiting + processes[idx].burst;
        processes[idx].ct = current_time;

        total_wt += processes[idx].waiting;
        total_tat += processes[idx].tat;
        is_completed[idx] = 1;
        scheduled++;
    }

    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(processes, n, results[0], results[1]);
}


void priority_preemptive(Process processes[], int n, float results[], GanttEntry gantt[], int *gantt_index) {
    Process temp[n];
    memcpy(temp, processes, sizeof(temp)); 

    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;
    int current_process = -1;
    int first_arrival = 9999;
 
    for (int i = 0; i < n; i++) {
        if (temp[i].arrival < first_arrival) {
            first_arrival = temp[i].arrival;
        }
    }
    current_time = first_arrival;

    while (completed < n) {
        int new_arrival = 0;
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival == current_time) {
                new_arrival = 1;
                break;
            }
        }
 
        int highest_priority = -1;
        int min_priority = 9999;
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= current_time && temp[i].remaining_bt > 0 && 
                temp[i].priority < min_priority) {
                highest_priority = i;
                min_priority = temp[i].priority;
            } else if (temp[i].arrival <= current_time && temp[i].remaining_bt > 0 && 
                      temp[i].priority == min_priority) {
                if (current_process != -1 && i == current_process) {
                    highest_priority = i;
                } else if (highest_priority == -1 || temp[i].id < temp[highest_priority].id) {
                    highest_priority = i;
                }
            }
        }

        if (highest_priority == -1) {
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
                current_process = -1;
            }
            current_time++;
            continue;
        }
 
        if (current_process != highest_priority || new_arrival) {
            if (current_process != -1) {
                gantt[*gantt_index].end_time = current_time;
                (*gantt_index)++;
            }
            gantt[*gantt_index].process_id = temp[highest_priority].id;
            gantt[*gantt_index].start_time = current_time;
            current_process = highest_priority;
        }
 
        temp[highest_priority].remaining_bt--;
        current_time++;
 
        if (temp[highest_priority].remaining_bt == 0) {
            temp[highest_priority].tat = current_time - temp[highest_priority].arrival;
            temp[highest_priority].waiting = temp[highest_priority].tat - temp[highest_priority].burst;
            temp[highest_priority].ct = current_time;
            total_wt += temp[highest_priority].waiting;
            total_tat += temp[highest_priority].tat;
            completed++;
            current_process = -1;
            gantt[*gantt_index].end_time = current_time;
            (*gantt_index)++;
        }
    }

  
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (processes[i].id == temp[j].id) {
                processes[i].ct = temp[j].ct;
                processes[i].waiting = temp[j].waiting;
                processes[i].tat = temp[j].tat;
                break;
            }
        }
    }

    results[0] = total_wt / n;
    results[1] = total_tat / n;
    display_gantt_chart(gantt, *gantt_index);
    print_results(processes, n, results[0], results[1]); 
}


int main() {
    int choice, sub_choice, n, quantum = 1;  
    GanttEntry gantt[100];
    int gantt_index;
    
    while (1) { // Loop for "Run Again?"
         
        printf("\n--- Intelligent CPU Scheduler Simulator ---\n");
        printf("Scheduling Algorithms:\n");
        printf("1. FCFS\n2. SJF\n3. Round Robin\n4. Priority\n5. Exit\n");
        printf("Enter choice (1-5): ");
        
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number between 1 and 5.\n");
             
            while (getchar() != '\n');
            continue;
        }
       
        while (getchar() != '\n');
        
        if (choice == 5) {
            break;
        }
        
        if (choice < 1 || choice > 5) {
            printf("Invalid choice! Please select a number between 1 and 5.\n");
            continue;
        }

         
        int algo_choice = choice;  
        if (choice == 2 || choice == 4) {
            printf("\nSelect mode:\n");
            printf("1. Non-Preemptive\n2. Preemptive\n");
            printf("Enter choice (1-2): ");
            
            if (scanf("%d", &sub_choice) != 1) {
                printf("Invalid input! Please enter a number between 1 and 2.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');
            
            if (sub_choice < 1 || sub_choice > 2) {
                printf("Invalid choice! Please select a number between 1 and 2.\n");
                continue;
            }
        }

     
        printf("Enter number of processes: ");
        if (scanf("%d", &n) != 1) {
            printf("Invalid input! Please enter a valid number of processes.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        
        Process processes[n];
        input_processes(processes, n, algo_choice);
         
        if (choice == 3) {
            printf("Enter time quantum for Round Robin: ");
            if (scanf("%d", &quantum) != 1) {
                printf("Invalid input! Please enter a valid time quantum.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');
        }
 
        if (!validate_inputs(processes, n, quantum)) {
            continue;  
        }
 
        gantt_index = 0;
        float results[2];
        Process copy[n];
        reset_processes(copy, processes, n);

        switch (choice) {
            case 1:
                printf("\n--- FCFS Scheduling ---\n");
                fcfs(copy, n, results, gantt, &gantt_index);
                break;
            case 2:
                if (sub_choice == 1) {
                    printf("\n--- SJF (Non-Preemptive) Scheduling ---\n");
                    sjf_non_preemptive(copy, n, results, gantt, &gantt_index);
                } else {
                    printf("\n--- SJF Preemptive (SRTF) Scheduling ---\n");
                    sjf_preemptive(copy, n, results, gantt, &gantt_index);
                }
                break;
            case 3:
                printf("\n--- Round Robin Scheduling ---\n");
                rr(copy, n, results, gantt, &gantt_index, quantum);
                break;
            case 4:
                if (sub_choice == 1) {
                    printf("\n--- Priority (Non-Preemptive) Scheduling ---\n");
                    priority_non_preemptive(copy, n, results, gantt, &gantt_index);
                } else {
                    printf("\n--- Priority Preemptive Scheduling ---\n");
                    priority_preemptive(copy, n, results, gantt, &gantt_index);
                }
                break;
        }
 
        char run_again;
        printf("\nDo you want to run another simulation? (y/n): ");
        scanf(" %c", &run_again);
        while (getchar() != '\n'); // Clear buffer
        if (run_again != 'y' && run_again != 'Y') {
            break;
        }
    }
    printf("Thank you for using the CPU Scheduler Simulator!\n");
    return 0;
}