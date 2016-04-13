[![Build Status](https://drone.io/github.com/enukane/josh/status.png)](https://drone.io/github.com/enukane/josh/latest)
# josh

json output shell command inspired by jpmens/jo, but it's based on jansson and MIT License.

- Almost same interface as jo
- Won't accept stdin.
- No one-line nested element (using '[]' to express hash)

pretty print
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
