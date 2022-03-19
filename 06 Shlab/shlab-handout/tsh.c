/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

/* My additional helper routines (check the return value of system calls) */
pid_t Fork();
void Sigfillset(sigset_t *set);
void Sigemptyset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Setpgid(pid_t pid, pid_t pgid);
void Kill(pid_t pid, int sig);

/* Mine ends here */

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}

	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
        fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
    eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS];    // Argument list execve()
    pid_t pid;              // Process id

    int bg = parseline(cmdline, argv); // Backgruond flag, get 1 when the process run in background and 0 otherwise.

    if (argv[0] == NULL) {  // Ignore empty lines
        return;
    }
    
    if (!builtin_cmd(argv)) { // If not a builtin command, then need to run a program in argv[0]
        
        sigset_t mask_all, prev_all, mask_one, prev_one;
        Sigfillset(&mask_all);          // Create a mask that block all signals
        Sigemptyset(&mask_one);         
        Sigaddset(&mask_one, SIGCHLD);  // Create a mask that only block SIGCHLD

        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);   // Block SIGCHLD to avoid the race between addjob and deletejob
        if ((pid = Fork()) == 0) {
            /* Child process */
            Sigprocmask(SIG_SETMASK, &prev_one, NULL);  // Unblock SIGCHLD
            Setpgid(0, 0);         // Set the pgid equal to child's pid to avoid using the same pid with the shell.
            if (execve(argv[0], argv, environ) < 0) { // Child runs user job
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        /* Parent process */
        
        int state = bg? BG : FG;   // Get the job's state

        Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);   // Block all (because of accessing a shared global data structure)
        addjob(jobs, pid, state, cmdline);              // Add the child to the job list
        Sigprocmask(SIG_SETMASK, &prev_all, NULL);      // Restore previous mask
        
        if (!bg) {  // Wait for the foreground job to terminate or stop
            waitfg(pid);
        } else {    // Print the background job's information and return
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);      // Optionally unblock SIGCHLD
    }

    
    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if (!strcmp(argv[0], "quit")) {     // quit command
        exit(0);
    }
    if (!strcmp(argv[0], "&")) {        // Ignore singleton &
        return 1;
    }
    if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg")) {     // bg or fg command
        do_bgfg(argv);
        return 1;
    }
    if (!strcmp(argv[0], "jobs")) {     // jobs command
        listjobs(jobs);
        return 1;
    }

    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    if (argv[1] == NULL) {
        printf("%s command requires PID or %cjobid argument\n", argv[0], '%');
        return;
    }
    char ch = argv[1][0];   // The first char of argv[1]
    struct job_t *jobptr;   // Pointer to a job
    int num;                // Numbers convert from argv[1]

    sigset_t mask_all, prev_all;
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_all); // Block all (because of accessing a shared global data structure)

    if (ch == '%') {        // If the first char is '%', then match the jid
        num = atoi(argv[1] + 1);
        if ((jobptr = getjobjid(jobs, num)) == NULL) {
            printf("%s: No such job\n", argv[1]);
            return;
        }
    } else {                // Else match the pid
        if (!isdigit(ch)) {
            printf("%s: argument must be a PID or %cjobid\n", argv[0], '%');
            return;
        }
        num = atoi(argv[1]);
        if ((jobptr = getjobpid(jobs, num)) == NULL) {
            printf("(%s): No such process\n", argv[1]);
            return;
        }        
    }

    Kill(-(jobptr->pid), SIGCONT);  // Send SIGCONT to the matching job's processes
    if (!strcmp(argv[0], "bg")) {   // bg command
        jobptr->state = BG;
        printf("[%d] (%d) %s",jobptr->jid ,jobptr->pid, jobptr->cmdline);
    } else {                        // fg command
        jobptr->state = FG;
        waitfg(jobptr->pid);
    }

    Sigprocmask(SIG_SETMASK, &prev_all, NULL);    // Restore previous mask
    
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {
    sigset_t mask_empty;
    Sigemptyset(&mask_empty);       // Create an empty mask
    while (fgpid(jobs) > 0) {       // If there exists a foreground job
        sigsuspend(&mask_empty);    // Temporarily unblock all signals ->  pause()  ->  restore previous mask
    }
    
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    int status;
    pid_t pid;
   
    Sigfillset(&mask_all);
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Sigprocmask(SIG_BLOCK, &mask_all, &prev_all); // Block all (because of accessing a shared global data structure)
        if (WIFEXITED(status)) {   // If the process terminates normally (by itself)         
            deletejob(jobs, pid);  // Delete the child from the job list

        } else if (WIFSIGNALED(status)) { // If the process terminates by a signal
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
            deletejob(jobs, pid);   // Delete the child from the job list

        } else if (WIFSTOPPED(status)){ // If the process stops by a signal
            struct job_t *job = getjobpid(jobs, pid);
            printf("Job [%d] (%d) stopped by signal %d\n", job->jid, pid, WSTOPSIG(status));
            job->state = ST;

        } else { // Control never reaches here
            unix_error("waitpid unknown condition");
        }
        Sigprocmask(SIG_SETMASK, &prev_all, NULL);    // Restore previous mask
    }

    errno = olderrno;
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    int olderrno = errno;

    sigset_t mask_all, prev_all;
    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_all); // Block all (because of accessing a shared global data structure)

    pid_t fg_pid = fgpid(jobs);
    if (fg_pid != 0) { // If there is a foreground job
        Kill(-fg_pid, SIGINT); // Send SIGINT to all of the job's processes
    }

    Sigprocmask(SIG_SETMASK, &prev_all, NULL);    // Restore previous mask
    
    errno = olderrno;
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    int olderrno = errno;

    sigset_t mask_all, prev_all;
    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_all); // Block all (because of accessing a shared global data structure)

    pid_t fg_pid = fgpid(jobs);
    if (fg_pid != 0) { // If there is a foreground job
        Kill(-fg_pid, SIGTSTP); // Send SIGTSTP to all of the job's processes
    }

    Sigprocmask(SIG_SETMASK, &prev_all, NULL);    // Restore previous mask
    
    errno = olderrno;
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* My additional helper routines (check the return value of system calls) */
pid_t Fork() {
    pid_t pid = fork();
    if (pid < 0) {
        unix_error("Fork error");
    }
    return pid;
}

void Sigfillset(sigset_t *set) {
    if (sigfillset(set) < 0) {
        unix_error("Sigfillset error");
    }
    return;
}

void Sigemptyset(sigset_t *set) {
    if (sigemptyset(set) < 0) {
        unix_error("Sigemptyset error");
    }
    return;
}

void Sigaddset(sigset_t *set, int signum) {
    if (sigaddset(set, signum) < 0) {
        unix_error("Sigaddset error");
    }
    return;
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (sigprocmask(how, set, oldset) < 0) {
        unix_error("Sigprocmask error");
    }
    return;
}

void Setpgid(pid_t pid, pid_t pgid) {
    int rc;
    if ((rc = setpgid(pid, pgid)) < 0) {
        unix_error("Setpgid error");
    }
    return;
}

void Kill(pid_t pid, int sig) {
    int rc;
    if ((rc = kill(pid, sig)) < 0) {
        unix_error("Kill error");
    }
    return;
}
/* Mine ends here */


/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    Sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



