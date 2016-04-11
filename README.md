# josh

json output shell command inspired by jpmens/jo, but it's tiny and MIT License.

- Almost same interface as jo
- Won't accept stdin.
- No nested element

pretty pring
```
% josh -p name=josh year=99 active=true
{
  "name":"josh",
  "year":99,
  "active":true
}
```

array
```
% josh -a 1 2 3 4 5 6 7 8 9 10
[1,2,3,4,5,6,7,8,9,10]
```

combine array into object
```
% josh -p name=josh year=99 active=true score=`josh -a 1 2 3 4 5 6 7 8 9 10`
{
  "name":"josh",
  "year":99,
  "active":true,
  "score":[
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10
  ]
}
```
