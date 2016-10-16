# Vulkan & C++11 

This will be a test bed to try out new C++11 (and maybe 14) features. The goal is render and simulate falling cubes with the bear minimum calls being handled on the main thread. 

We'll have a Threadpool which will accept a function pointer with any number arguments and returns a std::future, using the future's we can know when we can continue. I also want to implement pausing a task and continue doing it later form the same thread.



