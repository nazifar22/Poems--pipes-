# Poems via Pipe

I developed a C program that simulates an Easter-themed poem-sharing activity between Mama Bunny (the parent process) and her four bunny boys (child processes). Mama Bunny stores watering poems in a file and allows interaction through a menu-based interface. A new option, “Watering”, was added to this menu, which triggers the selection of one of the bunny boys at random using fork().

Once selected, the child (bunny boy) sends a signal to the parent to indicate his arrival in Barátfa. Mama Bunny responds by sending two randomly chosen poems to the child through a pipe. The child displays these poems, picks one at random, and sends the choice back to Mama Bunny through a message queue (for logging or acknowledgment). The child then simulates reciting the chosen poem by printing it along with the message “May I water!”, completes the watering task, and returns to the terminal (ends execution).

The implementation uses UNIX inter-process communication mechanisms, including signals, pipes, and System V message queues, and ensures proper handling of child processes. It’s designed to run on Linux systems and demonstrates key concepts in process creation, IPC, random selection, and file management in C.

How to Use:
Compile the program:
  gcc mama_bunny_water.c -o bunny_water -lrt  
Run the program:
  ./bunny_water  
From the menu, choose to:
Add/list/modify/delete poems
Start the “Watering” simulation
Exit
When “Watering” is selected:
A bunny boy is chosen at random
Mama sends 2 poems via pipe
Bunny picks one, says “May I water!”, then exits
Control returns to Mama Bunny (terminal)
