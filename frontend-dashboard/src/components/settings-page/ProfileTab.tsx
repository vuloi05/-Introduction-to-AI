import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'

export default function ProfileTab() {
  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <Card>
        <CardHeader>
          <CardTitle>Profile</CardTitle>
          <p className="text-sm text-muted-foreground">This is how others will see you on the site.</p>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">Username</label>
            <input
              className="flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm transition-colors placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              placeholder="acme_admin"
              defaultValue="tuanm"
            />
            <p className="text-[0.8rem] text-muted-foreground">This is your public display name.</p>
          </div>

          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">Email</label>
            <input
              type="email"
              className="flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm transition-colors placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              defaultValue="tuanm@example.com"
            />
            <p className="text-[0.8rem] text-muted-foreground">You can manage verified email addresses in your email settings.</p>
          </div>

          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">Bio</label>
            <textarea
              className="flex min-h-[80px] w-full rounded-md border border-input bg-transparent px-3 py-2 text-sm shadow-sm placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              placeholder="Tell us a little bit about yourself"
              defaultValue="I'm a senior frontend developer who loves building beautiful UIs."
            />
          </div>
        </CardContent>
        <div className="flex items-center p-6 pt-0">
            <Button>Update profile</Button>
        </div>
      </Card>
    </div>
  )
}
