testing()
testing()
local t1 = testObj('test1')
local t2 = testObj('test2')
local t3 = t1..t2

print(t1, type(t1), getmetatable(t1), t1.value)
print(t2, type(t2), getmetatable(t2), t2.value)
print(t3, type(t3), getmetatable(t3), t3.value)
t3.value = "Halelujah!"
print(t3, type(t3), getmetatable(t3), t3.value, t3.method())
