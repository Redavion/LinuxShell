# spring2017-assignment6

## Due Dates: Intial Submission for Feedback on April 6, Final Submission due on April 13

In this assignment you will extend your shell from assignment 3 and enable the following commands. 

- Builtin command:   
  -  set policy [FIFO|RR|OTHER] for the tasks
  -  get policy
  -  set priority : it should fail if the priority is not within the range for the policy
  - get priority
- Enable piped execution of foreground jobs
  - Task1|Task2|Task3|… (max 10 tasks) should enable the pipe as shown in the ls –l |less example (see examples repository)
- The  execution of this code will be tested manually. You will be asked to describe how you tested your code in a report [to be checked in as report.md]
- Grading criteria [17 points each for all five implementations] + 5 points for style and documentation + 10 points for testing and report.
- There is no travis integration required.
 
 To begin this assignment, copy the code from your assignment 3 repository into the new repository and implement the new functionality.
