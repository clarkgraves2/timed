# Project Summary
Created a daytime protocol server. A client can connect and request the time. An additional feature is that the client can optionally request the time in a certain formatted supported with strftime() function. The server can handle TCP and UDP requests, and can handle empty requests that act as a default time request.
# Challenges
1. The design of the project structure was the hardest part. I was initially going to implement a poll system with a thread system. After some discussion I came to the conclusion that it was overkill to implement a thread pool for this project.
2. Handling the TCP and UDP connections separately and the figuring out the logic for each. Even though this was a simple protocol to implement it still needed some careful consideration when handling each request so that the time could be correctly responded to.
3. Not using static global variables and instead learning how to properly implement structures that get passed to the modules functions instead of using global variables. Global variables are like hitting the easy not safe button.
# Successes
1. I was able to implement the project pretty efficiently in one day got it all implemented by carefully breaking up the modules to adhere to the single responsibility principle.
2. I'm getting better at managing my memory to where when I ran valgrind for the first time after getting the server ready to test it passed with no memory leaks first time.
3. I feel like I was ahead on this project to where I was able to logically think it through and complete it within the three days without getting stuck or stressing.
# Lessons Learned
1. Take more time to analyze the project requirements. I almost went down a rabbit hole and implemented a more complex solution to a simple problem.
2. If you modularize your project correctly and use name conventions that make your program clear it makes writing it easier. I was able to throughout the project know exactly what was happening with my program that made coding it simple.
3. That having a clear plan to test your code throughout the process makes for an easier completion of the project. I didn't have many errors to fix and no Valgrind errors. 
# Reflection
1. I'm trusting my coding abilities more that as long as I check, keep it simple, and do what I've been taught then I'll be able to complete the projects successfully. Keep learning, keep refining, and keep thinking critically.