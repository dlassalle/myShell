David Lassalle


IMPLEMENTATION DETAILS:
I started working on a single-file program and never found a place where there would be an advantage to having more than one file, so I kept this design. The bulk of our time was spent on parsing the commands that were entered, which had to be separated by new lines, semicolons, or ampersand, these individual commands needed to be separated into arguments. I then looked at the command, and if it was built in I executed it, otherwise I forked and exec'ed. I also checked I/O redirection and pipes and redirected appropriately. However, when I saved building the jobs command for last, we ended up hitting a roadblock that threatened to eat up our stretch days, so the decision was made to submit the assignment without it.

RESULTS:
We were able to make a working shell that could handle the specified built-in commands (aside from "jobs") as well as any commands in /bin in either the foreground or background. We could also execute commands with simple I/O redirection and any number of stringed together pipes. In the end, our work resulted in a simple shell that executed all commands and met all but one specifications assigned to us for this project. 


